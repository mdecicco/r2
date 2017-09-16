#ifndef SCENE_MANAGER
#define SCENE_MANAGER

#include <vector>
#include <string>
using namespace std;

#include <r2/utilities/mesh.h>

// hopefully such small default values will encourage vertex_format/index_type/instance_format pre-registration
#define DEFAULT_MAX_VERTICES  65536
#define DEFAULT_MAX_INDICES   65536
#define DEFAULT_MAX_INSTANCES 65536

namespace r2 {
    class render_node {
        public:
            render_node(const vtx_bo_segment& vertData,idx_bo_segment* indexData = nullptr,ins_bo_segment* instanceData = nullptr);
            ~render_node();

        protected:
            vtx_bo_segment m_vertexData;
            idx_bo_segment m_indexData;
            ins_bo_segment m_instanceData;
    };

    class r2engine;
    class scene;
    class scene_man {
        public:
            scene_man(r2engine* e);
            ~scene_man();

            r2engine* engine() const;

            scene* create(const string& name);
            void destroy(scene* s);

        protected:
            r2engine* m_eng;
            vector<scene*> m_scenes;
    };

    class scene {
        public:
            scene_man* manager() const;
            string name() const;

            bool operator==(const scene& rhs) const;

            template<typename vtx_type,typename idx_type=char,typename ins_type=char>
            render_node* add_mesh(render_mesh<vtx_type,idx_type,ins_type>* mesh) {
                if(!check_mesh(mesh->vertex_count())) return nullptr;

                vertex_buffer* target = nullptr;
                vtx_bo_segment vboData;

                // optional
                idx_bo_segment iboData;
                idx_bo_segment* iboDataPtr = nullptr;
                ins_bo_segment instanceData;
                ins_bo_segment* instanceDataPtr = nullptr;

                // vertices
                for(auto i = m_vtx_buffers.begin();i != m_vtx_buffers.end();i++) {
                    if((*i)->format() == mesh->vertexFormat()) {
                        target = (*i);
                    }
                }
                if(!target) {
                    m_vtx_buffers.push_back(new vertex_buffer(m_mgr->engine(),mesh->vertexFormat(),DEFAULT_MAX_VERTICES));
                    target = m_vtx_buffers.back();
                }

                vboData = target->append(mesh->vertex_data(),mesh->vertex_count());


                // indices
                if(mesh->index_count() > 0) {
                    index_buffer* itarget = nullptr;
                    for(auto i = m_idx_buffers.begin();i != m_idx_buffers.end();i++) {
                        if((*i)->type() == mesh->indexType()) {
                            itarget = (*i);
                        }
                    }
                    if(!itarget) {
                        m_idx_buffers.push_back(new index_buffer(m_mgr->engine(),mesh->indexType(),DEFAULT_MAX_INDICES));
                        itarget = m_idx_buffers.back();
                    }
                    iboData = itarget->append(mesh->index_data(),mesh->index_count());
                    iboDataPtr = &iboData;
                }

                // instances
                if(mesh->instance_count() > 0) {
                    instance_buffer* itarget = nullptr;
                    for(auto i = m_ins_buffers.begin();i != m_ins_buffers.end();i++) {
                        if((*i)->format() == mesh->instanceFormat()) {
                            itarget = (*i);
                        }
                    }
                    if(!itarget) {
                        m_ins_buffers.push_back(new instance_buffer(m_mgr->engine(),mesh->instanceFormat(),DEFAULT_MAX_INSTANCES));
                        itarget = m_ins_buffers.back();
                    }

                    instanceData = itarget->append(mesh->instance_data(),mesh->instance_count());
                    instanceDataPtr = &instanceData;
                }

                render_node* node = new render_node(vboData,iboDataPtr,instanceDataPtr);
                m_nodes.push_back(node);
                return node;
            }

        protected:
            friend class scene_man;
            scene(scene_man* m,const string& name);
            ~scene();
            bool check_mesh(size_t vc) const;

            scene_man* m_mgr;
            string m_name;

            // one buffer per vertex format
            vector<vertex_buffer*> m_vtx_buffers;
            // one buffer per index type
            vector<index_buffer*> m_idx_buffers;
            // one buffer per instance data format
            vector<instance_buffer*> m_ins_buffers;

            vector<render_node*> m_nodes;
    };
}

#endif /* end of include guard: SCENE_MANAGER */
