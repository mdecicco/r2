#pragma once
#include <r2/systems/entity.h>

namespace r2 {
	class transform_component : public scene_entity_component {
		public:
			transform_component();
			~transform_component();

			mat4f transform;
	};

	class transform_sys : public entity_system {
		public:
			transform_sys();
			~transform_sys();

			virtual const size_t component_size() const { return sizeof(transform_component); }
			virtual const size_t max_component_count() const { return 3; }

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

