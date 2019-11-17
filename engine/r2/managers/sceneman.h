#pragma once

#include <vector>
#include <unordered_map>
#include <string>
using namespace std;

#include <r2/utilities/mesh.h>
#include <r2/utilities/uniformbuffer.h>

#define DEFAULT_MAX_VERTICES		65536
#define DEFAULT_MAX_INDICES			65536
#define DEFAULT_MAX_INSTANCES		65536
#define DEFAULT_MAX_UNIFORM_BLOCKS	16384

namespace r2 {
	class node_material_instance;
    class render_node {
        public:
            render_node(const vtx_bo_segment& vertData, mesh_construction_data* cdata, idx_bo_segment* indexData = nullptr, ins_bo_segment* instanceData = nullptr);
            ~render_node();

			const vtx_bo_segment& vertices() const;
			const idx_bo_segment& indices() const;
			const ins_bo_segment& instances() const;

			node_material_instance* material;

        protected:
            vtx_bo_segment m_vertexData;
            idx_bo_segment m_indexData;
            ins_bo_segment m_instanceData;
			mesh_construction_data* m_constructionData;
    };

	class shader_program;
	class node_material_instance;
	class scene;
	class node_material {
		public:
			node_material(const string& shaderBlockName, const uniform_format& fmt);
			~node_material();

			void set_shader(shader_program* shader);
			shader_program* shader() const;
			const uniform_format& format() const;

			node_material_instance* instantiate(scene* s);

			string name;
		protected:
			uniform_format m_format;
			string m_shaderBlockName;
			shader_program* m_shader;
	};

	class node_material_instance {
		public:
			~node_material_instance();
			
			node_material* material() const;
			uniform_block* uniforms() const;

		protected:
			friend class node_material;
			node_material_instance(node_material* material, uniform_block* uniforms);

			node_material* m_material;
			uniform_block* m_uniforms;
	};

    class r2engine;
    class scene_man {
        public:
            scene_man(r2engine* e);
            ~scene_man();

            r2engine* engine() const;

            scene* create(const string& name);
			scene* get(const string& name);
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

            render_node* add_mesh(mesh_construction_data* mesh);
			uniform_block* allocate_uniform_block(const string& name, const uniform_format& fmt);

			void generate_vaos();
			void sync_buffers();
			void render();

        protected:
            friend class scene_man;
            scene(scene_man* m,const string& name);
            ~scene();
            bool check_mesh(size_t vc) const;

            scene_man* m_mgr;
            string m_name;

            // one buffer pool per vertex format
            unordered_map<string, buffer_pool> m_vtx_buffers;
            // one buffer pool per index type
			unordered_map<u8, buffer_pool> m_idx_buffers;
            // one buffer pool per instance data format
			unordered_map<string, buffer_pool> m_ins_buffers;
			// one buffer pool per uniform block format
			unordered_map<string, buffer_pool> m_ufm_buffers;

            vector<render_node*> m_nodes;
    };
};
