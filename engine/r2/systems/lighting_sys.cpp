#include <r2/systems/lighting_sys.h>
#include <r2/engine.h>


namespace r2 {
	lighting_component::lighting_component() {
		type = lt_point;
		color = vec3f(1.0f, 1.0f, 1.0f);
		coneInnerAngle = -1.0f;
		coneOuterAngle = -1.0f;
		constantAttenuation = 1.0f;
		linearAttenuation = 0.0f;
		quadraticAttenuation = 0.0f;
	}

	lighting_component::~lighting_component() {
	}

	void lighting_component::destroy() {
	}

	lighting_system_state::lighting_system_state() {
	}

	lighting_system_state::~lighting_system_state() {
	}



	lighting_system_state_factory::lighting_system_state_factory() {
	}

	lighting_system_state_factory::~lighting_system_state_factory() {
	}

	engine_state_data* lighting_system_state_factory::create() {
		return (engine_state_data*)new lighting_system_state();
	}


	lighting_sys* lighting_sys::instance = nullptr;
	lighting_sys::lighting_sys() {
	}

	lighting_sys::~lighting_sys() {
	}

	void lighting_sys::initialize_entity(scene_entity* entity) {
		if (!entity->is_scripted()) return;

		entity->bind(this, "add_light_component", [](entity_system* system, scene_entity* entity, v8Args args) {
			system->addComponentTo(entity);
		});
	}

	void lighting_sys::deinitialize_entity(scene_entity* entity) {
		if (!entity->is_scripted()) return;

		auto s = state();
		s.enable();
		if (!s->contains_entity(entity->id())) {
			entity->unbind("add_light_component");
		} else entity->unbind("lighting");
		s.disable();
	}

	scene_entity_component* lighting_sys::create_component(entityId id) {
		auto s = state();
		s.enable();
		auto out = s->create<lighting_component>(id);
		m_lightingState.enable();
		out->m_sysState = m_lightingState.get();
		m_lightingState.disable();
		s.disable();
		return out;
	}

	void lighting_sys::bind(scene_entity_component* component, scene_entity* entity) {
		using c = lighting_component;
		if (entity->is_scripted()) {
			entity->unbind("add_light_component");
			entity->bind(component, "lighting", "type", &c::type);
			entity->bind_interpolatable(component, "lighting", "color", &c::color);
			entity->bind_interpolatable(component, "lighting", "cone_inner_angle", &c::coneInnerAngle);
			entity->bind_interpolatable(component, "lighting", "cone_outer_angle", &c::coneOuterAngle);
			entity->bind_interpolatable(component, "lighting", "constant_attenuation", &c::constantAttenuation);
			entity->bind_interpolatable(component, "lighting", "linear_attenuation", &c::linearAttenuation);
			entity->bind_interpolatable(component, "lighting", "quadratic_attenuation", &c::quadraticAttenuation);
			entity->bind(this, "lighting", "remove", [](entity_system* system, scene_entity* entity, v8Args args) {
				system->removeComponentFrom(entity);
			});
		}

		entity->lighting = component_ref<c*>(this, component->id());
	}

	void lighting_sys::unbind(scene_entity* entity) {
		if (entity->is_scripted()) {
			entity->unbind("lighting");
			entity->bind(this, "add_light_component", [](entity_system* system, scene_entity* entity, v8Args args) {
				system->addComponentTo(entity);
			});
		}

		entity->lighting.clear();
	}

	void lighting_sys::initialize() {
		auto fac = new lighting_system_state_factory();
		auto stateMgr = r2engine::get()->states();
		m_lightingState = stateMgr->register_state_data_factory<lighting_system_state>(fac);

		r2engine::register_entity_property<light_type>("lighting.type");
		r2engine::register_entity_property<vec3f>("lighting.color");
		r2engine::register_entity_property<f32>("lighting.cone_inner_angle");
		r2engine::register_entity_property<f32>("lighting.cone_outer_angle");
		r2engine::register_entity_property<f32>("lighting.constant_attenuation");
		r2engine::register_entity_property<f32>("lighting.linear_attenuation");
		r2engine::register_entity_property<f32>("lighting.quadratic_attenuation");
	}

	void lighting_sys::tick(f32 dt) {
	}

	void lighting_sys::handle(event* evt) {
	}

	size_t lighting_sys::get_lights(size_t max_count, lighting_component** out) {
		auto s = state();
		s.enable();
		size_t c = 0;
		s->for_each<lighting_component>([max_count, &c, out](lighting_component* comp) {
			if (c == max_count) return false;

			out[c] = comp;

			c++;
			return true;
		});
		s.disable();

		return c;
	}
};