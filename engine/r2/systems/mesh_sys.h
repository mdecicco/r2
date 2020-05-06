#pragma once
#include <r2/systems/entity.h>
#include <r2/managers/sceneman.h>

namespace r2 {
	class mesh_component : public scene_entity_component {
		public:
			mesh_component();
			~mesh_component();

			void set_node(render_node* node);
			void release_node();
			render_node* get_node();

			void set_instance_data(v8Args args);
			void get_instance_data(v8Args args);
			void get_max_vertex_count(v8Args args);
			void get_vertex_count(v8Args args);
			void set_vertex_data(v8Args args);
			void get_vertex_data(v8Args args);
			void get_max_index_count(v8Args args);
			void get_index_count(v8Args args);
			void set_index_data(v8Args args);
			void get_index_data(v8Args args);

			template <typename instance_type>
			void set_instance_data(const instance_type& i) {
				if (!m_instance) {
					r2Error("Attempted to set mesh component's instance data when the mesh doesn't have a valid reference to a node");
					return;
				}

				m_instance.update_instance(i);
			}

			void set_instance_transform(const mat4f& transform);

			template <typename instance_type>
			instance_type* get_instance_data() {
				if (!m_instance) {
					r2Error("Attempted to get mesh component's instance data when the mesh doesn't have a valid reference to a node");
					return nullptr;
				}

				return (instance_type*)m_instance.node()->instance_data(m_instance.id());
			}

			size_t get_max_vertex_count();
			size_t get_vertex_count();

			template <typename vertex_type>
			void set_vertex_data(vertex_type* vertices, size_t count) {
				m_instance.update_vertices(vertices, count);
			}

			template <typename vertex_type>
			vertex_type* get_vertex_data() {
				if (!m_instance) {
					r2Error("Attempted to get mesh component's vertex data when the mesh doesn't have a valid reference to a node");
					return;
				}

				return (vertex_type*)m_instance.node()->vertex_data();
			}

			size_t get_max_index_count();
			size_t get_index_count();

			template <typename index_type>
			void set_index_data(index_type* indices, size_t count) {
				m_instance.update_indices(indices, count);
			}

			template <typename index_type>
			void get_index_data(index_type) {
				if (!m_instance) {
					r2Error("Attempted to get mesh component's index data when the mesh doesn't have a valid reference to a node");
					return;
				}

				return (index_type*)m_instance.node()->index_data();
			}

		protected:
			render_node_instance m_instance;
	};

	class mesh_sys : public entity_system {
		public:
			~mesh_sys();
			static mesh_sys* mesh_sys::get() {
				if (instance) return instance;
				instance = new mesh_sys();
				return instance;
			}

			virtual const size_t component_size() const { return sizeof(mesh_component); }

			virtual void initialize();
			virtual void initialize_entity(scene_entity* entity);
			virtual void deinitialize_entity(scene_entity* entity);
			virtual scene_entity_component* create_component(entityId id);
			virtual void bind(scene_entity_component* component, scene_entity* entity);
			virtual void unbind(scene_entity* entity);
			virtual void tick(f32 dt);
			virtual void handle(event* evt);

			void bind_instance_data(mesh_component* component, scene_entity* entity);
			void bind_vertex_data(mesh_component* component, scene_entity* entity);
			void bind_index_data(mesh_component* component, scene_entity* entity);
			void bind_node(mesh_component* component, scene_entity* entity);

		protected:
			mesh_sys();
			static mesh_sys* instance;
	};
};