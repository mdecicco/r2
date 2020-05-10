#include <r2/engine.h>
#include <r2/systems/transform_sys.h>
#include <r2/systems/cascade_functions.h>

namespace r2 {
	transform_component::transform_component() : transform(mat4f(1.0f)) {
	}

	transform_component::~transform_component() {
	}


	transform_sys* transform_sys::instance = nullptr;
	transform_sys::transform_sys() {
	}

	transform_sys::~transform_sys() {
	}

	void transform_sys::initialize_entity(scene_entity* entity) {
		if (!entity->is_scripted()) return;
		entity->bind(this, "add_transform_component", [](entity_system* system, scene_entity* entity, v8Args args) {
			system->addComponentTo(entity);
		});
	}
	void transform_sys::deinitialize_entity(scene_entity* entity) {
		if (!entity->is_scripted()) return;
		auto s = state();
		s.enable();
		if (!s->contains_entity(entity->id())) {
			entity->unbind("add_transform_component");
		}
		s.disable();
	}

	scene_entity_component* transform_sys::create_component(entityId id) {
		auto s = state();
		s.enable();
		auto out = s->create<transform_component>(id);
		s.disable();
		return out;
	}

	void transform_sys::bind(scene_entity_component* component, scene_entity* entity) {
		using c = transform_component;
		if (entity->is_scripted()) {
			entity->unbind("add_transform_component");
			entity->bind_interpolatable(component, "transform", &c::transform, true, &cascade_mat4f, "full_transform");
			entity->bind(this, "remove_transform_component", [](entity_system* system, scene_entity* entity, v8Args args) {
				system->removeComponentFrom(entity);
			});
		}
		entity->transform = component_ref<c*>(this, component->id());
	}
	void transform_sys::unbind(scene_entity* entity) {
		if (entity->is_scripted()) {
			entity->unbind("transform");
			entity->bind(this, "add_transform_component", [](entity_system* system, scene_entity* entity, v8Args args) {
				system->addComponentTo(entity);
			});
		}
		entity->transform.clear();
	}

	void transform_sys::initialize() {
		r2engine::register_entity_property<mat4f>("transform");
	}

	void transform_sys::tick(f32 dt) {
	}

	void transform_sys::handle(event* evt) {
	}
};
