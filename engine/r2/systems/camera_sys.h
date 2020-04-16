#pragma once
#include <r2/systems/entity.h>

namespace r2 {
	enum plane_index {
		pi_right,
		pi_left,
		pi_bottom,
		pi_top,
		pi_far,
		pi_near
	};

	class camera_frustum {
		public:
			struct frustum_plane {
				vec3f normal;
				f32 distance;
			};

			camera_frustum();
			~camera_frustum();

			void set(const mat4f& mvp);
			bool contains(const vec3f& point, f32 radius = -0.0f) const;

			frustum_plane planes[6];
	};

	class camera_component : public scene_entity_component {
		public:
			camera_component();
			~camera_component();

			void activate();
			inline bool is_active() const { return active; }
			void update_frustum();

			mat4f projection;
			camera_frustum frustum;

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