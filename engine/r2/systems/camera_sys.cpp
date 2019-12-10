#include <r2/engine.h>
#include <r2/systems/camera_sys.h>

namespace r2 {
	camera_component::camera_component() : projection(mat4f(1.0f)), active(false) {
	}

	camera_component::~camera_component() {
	}


	camera_sys::camera_sys() {
	}

	camera_sys::~camera_sys() {
	}

	void camera_sys::initialize_entity(scene_entity* entity) {
		entity->bind(this, "add_camera_component", [](entity_system* system, scene_entity* entity, v8Args args) {
			system->addComponentTo(entity);
		});
	}
	void camera_sys::deinitialize_entity(scene_entity* entity) {
		auto s = state();
		s.enable();
		if (!s->contains_entity(entity->id())) entity->unbind("add_camera_component");
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
		entity->unbind("add_camera_component");
		entity->bind(component, "projection", &c::projection);
		entity->bind(component, "active", &c::active, true);
		entity->bind(this, "activate", [](entity_system* sys, scene_entity* entity, v8Args args) {
			scene* curScene = r2engine::get()->current_scene();
			if (!curScene) {
				r2Error("There is no active scene for which to set an active camera");
				return;
			}

			auto state = sys->state();
			state.enable();
			camera_component* cam = (camera_component*)state->entity(entity->id());

			// do nothing if the camera is already active
			if (cam->active) {
				state.disable();
				return;
			}

			// set the active camera to inactive, if there is one
			state->for_each<camera_component>([](camera_component* c, size_t idx, bool& should_break) {
				// break if the active camera is found
				should_break = c->active;

				// deactivate
				c->active = false;
			});

			// activate this one
			cam->active = true;
			curScene->camera = entity;

			
			state.disable();
		});
		entity->bind(this, "remove_camera_component", [](entity_system* system, scene_entity* entity, v8Args args) {
			system->removeComponentFrom(entity);
		});
		entity->camera = component_ref<camera_component*>(this, component->id());
	}
	void camera_sys::unbind(scene_entity* entity) {
		entity->unbind("camera");
		entity->bind(this, "add_camera_component", [](entity_system* system, scene_entity* entity, v8Args args) {
			system->addComponentTo(entity);
		});
		
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
};
