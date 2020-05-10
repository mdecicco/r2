#pragma once
#include <r2/systems/entity.h>
#include <r2/systems/animation.h>

namespace r2 {
	class animation_component : public scene_entity_component {
		public:
			animation_component();
			~animation_component();

			dynamic_pod_array<animation_group*> animations;
	};

	class animation_sys : public entity_system, periodic_update {
		public:
			~animation_sys();
			static animation_sys* get() {
				if (instance) return instance;
				instance = new animation_sys();
				return instance;
			}

			virtual const size_t component_size() const { return sizeof(animation_component); }

			virtual void initialize();
			virtual void deinitialize();
			virtual void initialize_entity(scene_entity* entity);
			virtual void deinitialize_entity(scene_entity* entity);
			virtual scene_entity_component* create_component(entityId id);
			virtual void bind(scene_entity_component* component, scene_entity* entity);
			virtual void unbind(scene_entity* entity);
			virtual void tick(f32 dt);
			virtual void doUpdate(f32 frameDelta, f32 updateDelta);
			virtual void handle(event* evt);

		protected:
			animation_sys();
			static animation_sys* instance;
	};
};