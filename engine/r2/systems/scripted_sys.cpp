#include <r2/systems/scripted_sys.h>
#include <r2/engine.h>

namespace r2 {
	scripted_component::scripted_component() {
	}

	scripted_component::~scripted_component() {
	}


	scripted_sys::scripted_sys() {
	}

	scripted_sys::~scripted_sys() {
	}

	void scripted_sys::initialize_entity(scene_entity* entity) {
		if (!entity->is_scripted()) return;
		entity->bind(this, "add_scripted_component", [](entity_system* system, scene_entity* entity, v8Args args) {
			system->addComponentTo(entity);
		});
	}
	void scripted_sys::deinitialize_entity(scene_entity* entity) {
		if (!entity->is_scripted()) return;
		auto s = state();
		s.enable();
		if (!s->contains_entity(entity->id())) {
			entity->unbind("add_scripted_component");
		}
		s.disable();
	}

	scene_entity_component* scripted_sys::create_component(entityId id) {
		auto s = state();
		s.enable();
		auto out = s->create<scripted_component>(id);
		s.disable();
		return out;
	}

	void scripted_sys::bind(scene_entity_component* component, scene_entity* entity) {
		using c = scripted_component;
		if (entity->is_scripted()) {
			entity->unbind("add_scripted_component");
			
			// bind scripted component properties
			// entity->bind(component, "transform", &c::transform, false, true, &cascade_mat4f, "full_transform");
			
			entity->bind(this, "remove_scripted_component", [](entity_system* system, scene_entity* entity, v8Args args) {
				system->removeComponentFrom(entity);
			});
		}
	}
	void scripted_sys::unbind(scene_entity* entity) {
		if (entity->is_scripted()) {
			// unbind scripted component properties
			// entity->unbind("transform");
			entity->bind(this, "add_scripted_component", [](entity_system* system, scene_entity* entity, v8Args args) {
				system->addComponentTo(entity);
			});
		}
	}

	void scripted_sys::initialize() {
	}

	void scripted_sys::tick(f32 dt) {
	}

	void scripted_sys::handle(event* evt) {
	}
};
