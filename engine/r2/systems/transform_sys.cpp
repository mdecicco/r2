#include <r2/engine.h>
#include <r2/systems/transform_sys.h>

namespace r2 {
	transform_component::transform_component() {
	}

	transform_component::~transform_component() {
	}


	transform_sys::transform_sys() {
	}

	transform_sys::~transform_sys() {
	}

	void transform_sys::initialize_entity(scene_entity* entity) {
		entity->bind(this, "add_transform_component", [](entity_system* system, scene_entity* entity, v8Args args) {
			system->addComponentTo(entity);
		});
	}
	void transform_sys::deinitialize_entity(scene_entity* entity) {
		state().enable();
		if (!state()->contains_entity(entity->id())) entity->unbind("add_transform_component");
	}

	scene_entity_component* transform_sys::create_component(entityId id) {
		state().enable();
		auto out = state()->create<transform_component>(id);
		state().disable();
		return out;
	}

	void transform_sys::bind(scene_entity_component* component, scene_entity* entity) {
		using c = transform_component;
		entity->unbind("add_transform_component");
		entity->bind(this, "remove_transform_component", [](entity_system* system, scene_entity* entity, v8Args args) {
			system->removeComponentFrom(entity);
		});
	}
	void transform_sys::unbind(scene_entity* entity) {
		entity->bind(this, "add_transform_component", [](entity_system* system, scene_entity* entity, v8Args args) {
			system->addComponentTo(entity);
		});
	}

	void transform_sys::initialize() {
	}

	void transform_sys::tick(f32 dt) {
	}

	void transform_sys::handle(event* evt) {
	}
};
