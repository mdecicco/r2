#pragma once
#include <r2/systems/entity.h>

namespace r2 {
	class camera_component : public scene_entity_component {
		public:
			camera_component();
			~camera_component();

			void activate();
			inline bool is_active() const { return active; }

			mat4f projection;

		protected:
			friend class camera_sys;
			bool active;

	};

	class camera_sys : public entity_system {
		public:
			~camera_sys();
			static camera_sys* get() {
				if (instance) return instance;
				instance = new camera_sys();
				return instance;
			}

			virtual const size_t component_size() const { return sizeof(camera_component); }

			virtual void initialize();
			virtual void initialize_entity(scene_entity* entity);
			virtual void deinitialize_entity(scene_entity* entity);
			virtual scene_entity_component* create_component(entityId id);
			virtual void bind(scene_entity_component* component, scene_entity* entity);
			virtual void unbind(scene_entity* entity);
			virtual void tick(f32 dt);
			virtual void handle(event* evt);

			void activate_camera(scene_entity* entity);

		protected:
			camera_sys();
			static camera_sys* instance;
	};
};