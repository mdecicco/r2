#pragma once
#include <r2/systems/entity.h>

#include <r2/managers/memman.h>
#include <r2/bindings/v8helpers.h>

#include <r2/utilities/mesh.h>
#include <r2/utilities/uniformbuffer.h>
#include <r2/utilities/texture.h>

#define DEFAULT_MAX_VERTICES		65536
#define DEFAULT_MAX_INDICES			65536
#define DEFAULT_MAX_INSTANCES		65536
#define DEFAULT_MAX_UNIFORM_BLOCKS	16384

namespace r2 {
	typedef size_t instanceId;
	class render_node;
	class node_material_instance;
	class shader_program;
	class node_material_instance;
	class scene;

	enum primitive_type {
		pt_points = 0,
		pt_lines,
		pt_line_loop,
		pt_line_strip,
		pt_triangles,
		pt_triangle_strip,
		pt_triangle_fan,
		pt_quads,
		pt_quad_strip,
		pt_polygon
	};

	class render_node_instance {
		public:
			render_node_instance();
			render_node_instance(const render_node_instance& o);
			~render_node_instance();

			void release();
			operator bool() const;
			void update_instance_transform(const mat4f& transform);
			void update_instance_raw(const void* data);
			void update_vertices_raw(const void* data, size_t count);
			void update_indices_raw(const void* data, size_t count);
			inline render_node* node() { return m_node; }
			inline instanceId id() const { return m_id; }

			template <typename T>
			void update_instance(const T& data) {
				if (!m_node) {
					r2Error("Invalid render_node_instance cannot be updated");
					return;
				}
				assert(sizeof(T) == m_node->instances().buffer->format()->size());
				update_instance_raw(&data);
			}

			template <typename T>
			void update_vertices(const T* data, size_t count) {
				if (!m_node) {
					r2Error("Invalid render_node_instance cannot be updated");
					return;
				}
				assert(sizeof(T) == m_node->vertices().buffer->format()->size());
				update_vertices_raw(data, count);
			}

			template <typename T>
			void update_indices(const T* data, size_t count) {
				if (!m_node) {
					r2Error("Invalid render_node_instance cannot be updated");
					return;
				}
				assert(sizeof(T) == m_node->indices().buffer->type());
				update_indices_raw(data, count);
			}

		protected:
			friend class render_node;
			static instanceId nextInstanceId;
			render_node_instance(render_node* node);
			instanceId m_id;
			render_node* m_node;
	};

    class render_node {
        public:
            render_node(scene* s, const vtx_bo_segment& vertData, idx_bo_segment* indexData = nullptr, ins_bo_segment* instanceData = nullptr);
            ~render_node();

			const vtx_bo_segment& vertices() const;
			const idx_bo_segment& indices() const;
			const ins_bo_segment& instances() const;
			uniform_block* uniforms() const;

			void set_material_instance(node_material_instance* material);
			node_material_instance* material_instance() const;

			render_node_instance instantiate();
			void release(instanceId id);
			void update_instance_transform(instanceId id, const mat4f& transform);
			void update_instance_raw(instanceId id, const void* data);
			void* instance_data(instanceId id);
			void update_vertices_raw(const void* data, size_t count);
			void* vertex_data();
			void update_indices_raw(const void* data, size_t count);
			void* index_data();
			bool instance_valid(instanceId id) const;
			void set_vertex_count(size_t count);
			void set_index_count(size_t count);
			void add_uniform_block(uniform_block* uniforms);
			void remove_uniform_block(uniform_block* uniforms);
			const mlist<uniform_block*>& user_uniforms() const { return m_userUniforms; }
			inline size_t instance_count() const { return m_nextInstanceIdx; }
			inline size_t vertex_count() const { return m_vertexCount; }
			inline size_t index_count() const { return m_indexCount; }
			inline size_t max_instance_count() const { return m_instanceData.size(); }
			inline size_t max_vertex_count() const { return m_vertexData.size(); }
			inline size_t max_index_count() const { return m_indexData.size(); }

			template <typename T>
			void update_instance(instanceId id, const T& data) {
				assert(sizeof(T) == m_instanceData.buffer->format()->size());
				update_instance_raw(id, &data);
			}

			template <typename T>
			void update_vertices(instanceId id, const T* data, size_t count) {
				assert(sizeof(T) == m_vertexData.buffer->format()->size());
				update_vertices_raw(id, data, size_t count);
			}

			template <typename T>
			void update_indices(instanceId id, const T* data, size_t count) {
				assert(sizeof(T) == m_indexData.buffer->type());
				update_indices_raw(id, data, size_t count);
			}

			bool destroy_when_unused;
			bool has_transparency;
			primitive_type primitives;

        protected:
			friend class scene;
			scene* m_scene;
			uniform_block* m_uniforms;
			mlist<uniform_block*> m_userUniforms;
			node_material_instance* m_material;
            vtx_bo_segment m_vertexData;
            idx_bo_segment m_indexData;
            ins_bo_segment m_instanceData;
			size_t m_nextInstanceIdx;
			size_t m_vertexCount;
			size_t m_indexCount;
			munordered_map<instanceId, size_t> m_instanceIndices;
    };

	class node_material {
		public:
			node_material(const mstring& shaderBlockName = "", uniform_format* fmt = nullptr);
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
			struct texture_uniform {
				u32 location;
				u32 count;
				u32 currentFrame;
				f32 elapsed;
				f32 duration;
				bool loop;
				texture_buffer** textures;
			};

			~node_material_instance();
			
			node_material* material() const;
			uniform_block* uniforms() const;
			void uniforms_v8(v8::Local<v8::String> name, v8::PropertyCallbackInfo<v8::Value> const& info);

			void set_texture(const mstring& uniformName, texture_buffer* texture);
			void set_texture(const mstring& uniformName, const mvector<texture_buffer*>& textures, f32 duration, bool loop);
			inline u8 texture_count() const { return m_textures.size(); }
			inline texture_uniform* texture(u8 idx) { return &m_textures[idx]; }

		protected:
			friend class node_material;
			node_material_instance(node_material* material, uniform_block* uniforms);

			node_material* m_material;
			uniform_block* m_uniforms;
			mvector<texture_uniform> m_textures;
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
			texture_buffer* create_texture();
			render_buffer* create_render_target();

			void destroy(render_buffer* rbo);
			void destroy(shader_program* program);
			void destroy(texture_buffer* texture);

			void set_render_target(render_buffer* target);
			render_buffer* render_target();

			bool remove_node(render_node* node);

			void generate_vaos();
			void sync_buffers();
			void render(f32 dt);

			void release_resources();

			scene_entity* camera;

        protected:
            friend class scene_man;
            scene(scene_man* m,const mstring& name);
            ~scene();
            bool check_mesh(size_t vc) const;

            scene_man* m_mgr;
            mstring m_name;

			uniform_block* m_sceneUniforms;
			render_buffer* m_renderTarget;

            // one buffer pool per vertex format
            munordered_map<mstring, buffer_pool> m_vtx_buffers;
            // one buffer pool per index type
			munordered_map<u8, buffer_pool> m_idx_buffers;
            // one buffer pool per instance data format
			munordered_map<mstring, buffer_pool> m_ins_buffers;
			// one buffer pool per uniform block format
			munordered_map<mstring, buffer_pool> m_ufm_buffers;

			mvector<texture_buffer*> m_textures;
			mvector<render_buffer*> m_targets;
            mvector<render_node*> m_nodes;
			mvector<shader_program*> m_shaders;
    };
};
