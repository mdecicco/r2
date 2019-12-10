#pragma once
#include <r2/systems/entity.h>

namespace r2 {
	class camera_component : public scene_entity_component {
		public:
			camera_component();
			~camera_component();

			mat4f projection;
			bool active;
	};

	class camera_sys : public entity_system {
		public:
			camera_sys();
			~camera_sys();

			virtual const size_t component_size() const { return sizeof(camera_component); }

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