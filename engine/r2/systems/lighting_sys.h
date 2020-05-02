#pragma once
#include <r2/systems/entity.h>

namespace r2 {
	class lighting_system_state;

	enum light_type {
		lt_none,
		lt_point,
		lt_spot,
		lt_directional
	};

	class lighting_component : public scene_entity_component {
		public:
			lighting_component();
			~lighting_component();

			virtual void destroy();

			light_type type;
			vec3f color;
			f32 coneInnerAngle;
			f32 coneOuterAngle;
			f32 constantAttenuation;
			f32 linearAttenuation;
			f32 quadraticAttenuation;

		protected:
			friend class lighting_sys;
			lighting_system_state* m_sysState;
	};

	class lighting_system_state : public engine_state_data {
		public:
			lighting_system_state();
			~lighting_system_state();
	};

	class lighting_system_state_factory : public engine_state_data_factory {
		public:
			lighting_system_state_factory();
			~lighting_system_state_factory();

			virtual engine_state_data* create();
	};

	class lighting_sys : public entity_system {
		public:
			~lighting_sys();
			static lighting_sys* get() {
				if (instance) return instance;
				instance = new lighting_sys();
				return instance;
			}

			virtual const size_t component_size() const { return sizeof(lighting_component); }

			virtual void initialize();
			virtual void initialize_entity(scene_entity* entity);
			virtual void deinitialize_entity(scene_entity* entity);
			virtual scene_entity_component* create_component(entityId id);
			virtual void bind(scene_entity_component* component, scene_entity* entity);
			virtual void unbind(scene_entity* entity);
			virtual void tick(f32 dt);
			virtual void handle(event* evt);

			size_t get_lights(size_t max_count, lighting_component** out);

			engine_state_data_ref<lighting_system_state>& lightingState() { return m_lightingState; }


		protected:
			lighting_sys();
			static lighting_sys* instance;
			engine_state_data_ref<lighting_system_state> m_lightingState;
	};
};

