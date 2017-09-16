#ifndef MESH_DEFINITION
#define MESH_DEFINITION

#include <vector>
#include <string>
using namespace std;

#include <r2/utilities/indexbuffer.h>
#include <r2/utilities/vertexbuffer.h>
#include <r2/utilities/instancebuffer.h>

namespace r2 {
    class r2engine;

    template<typename vtx_type,typename idx_type,typename ins_type>
    class render_mesh {
        public:
            render_mesh(vertex_format* vtx_fmt,index_type idx_typ = it_byte,const instance_format* ins_fmt = nullptr) {
                m_vertexformat = vertex_format(*vtx_fmt);
                m_indexType = idx_typ;
                m_instanceFormat = instance_format(*ins_fmt);
            }
            ~render_mesh() { }

            vertex_format vertexFormat() const { return m_vertexformat; }
            void append_vertex(const vtx_type& v) { m_vertices.push_back(v); }
            size_t vertex_count() const { return m_vertices.size(); }
            const void* vertex_data() const { return &m_vertices[0]; }

            index_type indexType() const { return m_indexType; }
            void append_index(const idx_type& i) { m_indices.push_back(i); }
            size_t index_count() const { return m_indices.size(); }
            const void* index_data() const { return &m_indices[0]; }

            instance_format instanceFormat() const { return m_instanceFormat; }
            void append_instance(const ins_type& i) { m_instances.push_back(i); }
            size_t instance_count() const { return m_instances.size(); }
            const void* instance_data() const { return &m_instances[0]; }


        protected:
            //not optional
            vertex_format m_vertexformat;
            vector<vtx_type> m_vertices;

            //optional
            index_type m_indexType;
            vector<idx_type> m_indices;
            instance_format m_instanceFormat;
            vector<ins_type> m_instances;
    };
}

#endif /* end of include guard: MESH_DEFINITION */
