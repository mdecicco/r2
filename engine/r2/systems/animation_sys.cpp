#include <r2/systems/animation_sys.h>
#include <r2/engine.h>

namespace r2 {
	animation_component::animation_component() {
	}

	animation_component::~animation_component() {
	}


	animation_sys* animation_sys::instance = nullptr;
	animation_sys::animation_sys() {
	}

	animation_sys::~animation_sys() {
	}

	void animation_sys::initialize_entity(scene_entity* entity) {
		if (!entity->is_scripted()) return;
		entity->bind(this, "add_animation_component", [](entity_system* system, scene_entity* entity, v8Args args) {
			system->addComponentTo(entity);
		});
	}
	void animation_sys::deinitialize_entity(scene_entity* entity) {
		if (!entity->is_scripted()) return;
		auto s = state();
		s.enable();
		if (!s->contains_entity(entity->id())) entity->unbind("animation");
		else entity->unbind("add_animation_component");
		s.disable();
	}

	scene_entity_component* animation_sys::create_component(entityId id) {
		auto s = state();
		s.enable();
		auto out = s->create<animation_component>(id);
		s.disable();
		return out;
	}

	void animation_sys::bind(scene_entity_component* component, scene_entity* entity) {
		using c = animation_component;
		if (entity->is_scripted()) {
			entity->unbind("add_animation_component");
			entity->bind(this, "animation", "remove", [](entity_system* system, scene_entity* entity, v8Args args) {
				system->removeComponentFrom(entity);
			});
		}
		entity->animation = component_ref<c*>(this, component->id());
	}
	void animation_sys::unbind(scene_entity* entity) {
		if (entity->is_scripted()) {
			entity->bind(this, "add_animation_component", [](entity_system* system, scene_entity* entity, v8Args args) {
				system->addComponentTo(entity);
			});
		}
		entity->animation.clear();
	}

	void animation_sys::initialize() {
		initialize_periodic_update();
		setUpdateFrequency(60.0f);
		start_periodic_updates();
	}

	void animation_sys::deinitialize() {
		destroy_periodic_update();
	}

	void animation_sys::tick(f32 dt) {
		update(dt);
	}

	void animation_sys::doUpdate(f32 frameDelta, f32 updateDelta) {
		auto& state = this->state();
		state.enable();
		state->for_each<animation_component>([updateDelta](animation_component* comp) {
			scene_entity* target = comp->entity();
			comp->animations.for_each([target, updateDelta](animation_group** anim) {
				(*anim)->update(updateDelta, target);
				return true;
			});

			return true;
		});
		state.disable();
	}

	void animation_sys::handle(event* evt) {
	}
};
