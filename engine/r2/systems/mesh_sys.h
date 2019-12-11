#pragma once
#include <r2/systems/entity.h>
#include <r2/managers/sceneman.h>

namespace r2 {
	class mesh_component : public scene_entity_component {
		public:
			mesh_component();
			~mesh_component();

			void set_node(render_node* node);
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

		protected:
			render_node_instance m_instance;
	};

	class mesh_sys : public entity_system {
		public:
			mesh_sys();
			~mesh_sys();

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
	};
};