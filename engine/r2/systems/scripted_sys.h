#pragma once
#include <r2/systems/entity.h>

namespace r2 {
	class scripted_component : public scene_entity_component {
		public:
			scripted_component();
			~scripted_component();

			mat4f transform;
	};

	class scripted_sys : public entity_system {
		public:
			scripted_sys();
			~scripted_sys();

			virtual const size_t component_size() const;

			virtual void initialize();
			virtual void initialize_entity(scene_entity* entity);
			virtual void deinitialize_entity(scene_entity* entity);
			virtual scene_entity_component* create_component(entityId id);
			virtual void bind(scene_entity_component* component, scene_entity* entity);
			virtual void unbind(scene_entity* entity);
			virtual void tick(f32 dt);
			virtual void handle(event* evt);
	};
};