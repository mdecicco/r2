#include <r2/engine.h>
#include <r2/systems/camera_sys.h>

namespace r2 {
	camera_component::camera_component() : projection(mat4f(1.0f)), active(false) {
	}

	camera_component::~camera_component() {
	}

	void camera_component::activate() {
		((camera_sys*)system())->activate_camera(entity());
	}

	camera_sys* camera_sys::instance = nullptr;
	camera_sys::camera_sys() {
	}

	camera_sys::~camera_sys() {
	}

	void camera_sys::initialize_entity(scene_entity* entity) {
		if (!entity->is_scripted()) return;
		entity->bind(this, "add_camera_component", [](entity_system* system, scene_entity* entity, v8Args args) {
			system->addComponentTo(entity);
		});
	}

	void camera_sys::deinitialize_entity(scene_entity* entity) {
		if (!entity->is_scripted()) return;
		auto s = state();
		s.enable();
		if (!s->contains_entity(entity->id())) {
			entity->unbind("add_camera_component");
		}
		s.disable();
	}

	scene_entity_component* camera_sys::create_component(entityId id) {
		auto s = state();
		s.enable();
		auto out = s->create<camera_component>(id);
		s.disable();
		return out;
	}

	void camera_sys::bind(scene_entity_component* component, scene_entity* entity) {
		using c = camera_component;
		if (entity->is_scripted()) {
			entity->unbind("add_camera_component");

			entity->bind(component, "projection", &c::projection);
			entity->bind(component, "active", &c::active, true);
			entity->bind(this, "activate", [](entity_system* sys, scene_entity* entity, v8Args args) {
				((camera_sys*)sys)->activate_camera(entity);
			});
			entity->bind(this, "remove_camera_component", [](entity_system* system, scene_entity* entity, v8Args args) {
				system->removeComponentFrom(entity);
			});
		}
		entity->camera = component_ref<c*>(this, component->id());
	}

	void camera_sys::unbind(scene_entity* entity) {
		if (entity->is_scripted()) {
			entity->unbind("camera");
			entity->bind(this, "add_camera_component", [](entity_system* system, scene_entity* entity, v8Args args) {
				system->addComponentTo(entity);
			});
		}
		
		scene* curScene = r2engine::get()->current_scene();
		if (curScene && curScene->camera == entity) {
			r2Warn("Camera component removed from active camera. The scene will now have no active camera");
			curScene->camera = nullptr;
		}

		entity->camera.clear();
	}

	void camera_sys::initialize() {
	}

	void camera_sys::tick(f32 dt) {
	}

	void camera_sys::handle(event* evt) {
	}

	void camera_sys::activate_camera(scene_entity* entity) {
		if (!entity->camera) {
			r2Error("Entity \"%s\" has no camera component", entity->name().c_str());
			return;
		}

		scene* curScene = r2engine::get()->current_scene();
		if (!curScene) {
			r2Error("There is no active scene for which to set an active camera");
			return;
		}

		auto state = this->state();
		state.enable();
		camera_component* cam = entity->camera.get();

		// do nothing if the camera is already active
		if (cam->active) {
			state.disable();
			return;
		}

		// set the active camera to inactive, if there is one
		state->for_each<camera_component>([](camera_component* c) {
			bool should_break = c->active;

			// deactivate
			c->active = false;

			// break if the active camera is found
			return !should_break;
		});

		// activate this one
		cam->active = true;
		curScene->camera = entity;
		state.disable();
	}
};
