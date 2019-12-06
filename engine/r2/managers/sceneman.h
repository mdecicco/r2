#pragma once

#include <r2/managers/memman.h>
#include <r2/bindings/v8helpers.h>

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

			void set_material_instance(node_material_instance* material);
			node_material_instance* material_instance() const;

        protected:
			friend class scene;
			uniform_block* m_uniforms;
			node_material_instance* m_material;
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
			node_material(const mstring& shaderBlockName, uniform_format* fmt);
			~node_material();

			void set_shader(shader_program* shader);
			shader_program* shader() const;
			uniform_format* format() const;

			node_material_instance* instantiate(scene* s);

			mstring name;
		protected:
			uniform_format* m_format;
			mstring m_shaderBlockName;
			shader_program* m_shader;
	};

	class node_material_instance {
		public:
			~node_material_instance();
			
			node_material* material() const;
			uniform_block* uniforms() const;
			void uniforms_v8(v8::Local<v8::String> name, v8::PropertyCallbackInfo<v8::Value> const& info);

		protected:
			friend class node_material;
			node_material_instance(node_material* material, uniform_block* uniforms);

			node_material* m_material;
			uniform_block* m_uniforms;
	};

    class scene_man {
        public:
            scene_man();
            ~scene_man();

            scene* create(const mstring& name);
			scene* get(const mstring& name);
            void destroy(scene* s);

        protected:
            mvector<scene*> m_scenes;
    };

    class scene {
        public:
            scene_man* manager() const;
			const mstring& name() const;

            bool operator==(const scene& rhs) const;

            render_node* add_mesh(mesh_construction_data* mesh);
			uniform_block* allocate_uniform_block(const mstring& name, uniform_format* fmt);
			shader_program* load_shader(const mstring& file, const mstring& assetName);

			void generate_vaos();
			void sync_buffers();
			void render();

			void release_resources();

        protected:
            friend class scene_man;
            scene(scene_man* m,const mstring& name);
            ~scene();
            bool check_mesh(size_t vc) const;

            scene_man* m_mgr;
            mstring m_name;

			uniform_block* m_sceneUniforms;

            // one buffer pool per vertex format
            munordered_map<mstring, buffer_pool> m_vtx_buffers;
            // one buffer pool per index type
			munordered_map<u8, buffer_pool> m_idx_buffers;
            // one buffer pool per instance data format
			munordered_map<mstring, buffer_pool> m_ins_buffers;
			// one buffer pool per uniform block format
			munordered_map<mstring, buffer_pool> m_ufm_buffers;

            mvector<render_node*> m_nodes;
			mvector<shader_program*> m_shaders;
    };
};
