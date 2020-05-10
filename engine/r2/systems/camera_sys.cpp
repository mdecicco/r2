#include <r2/engine.h>
#include <r2/systems/camera_sys.h>
#include <r2/systems/cascade_functions.h>

namespace r2 {
	camera_frustum::camera_frustum() {
	}

	camera_frustum::~camera_frustum() {
	}

	void camera_frustum::set(const mat4f& mvp) {
		planes[pi_right] = {
			vec3f(
				mvp[0][3] + mvp[0][0],
				mvp[1][3] + mvp[1][0],
				mvp[2][3] + mvp[2][0]
			),
			mvp[3][3] + mvp[3][0]
		};

		planes[pi_left] = {
			vec3f(
				mvp[0][3] - mvp[0][0],
				mvp[1][3] - mvp[1][0],
				mvp[2][3] - mvp[2][0]
			),
			mvp[3][3] - mvp[3][0]
		};

		planes[pi_bottom] = {
			vec3f(
				mvp[0][3] + mvp[0][1],
				mvp[1][3] + mvp[1][1],
				mvp[2][3] + mvp[2][1]
			),
			mvp[3][3] + mvp[3][1]
		};

		planes[pi_top] = {
			vec3f(
				mvp[0][3] - mvp[0][1],
				mvp[1][3] - mvp[1][1],
				mvp[2][3] - mvp[2][1]
			),
			mvp[3][3] - mvp[3][1]
		};

		planes[pi_far] = {
			vec3f(
				mvp[0][2],
				mvp[1][2],
				mvp[2][2]
			),
			mvp[3][2]
		};

		planes[pi_near] = {
			vec3f(
				mvp[0][3] - mvp[0][2],
				mvp[1][3] - mvp[1][2],
				mvp[2][3] - mvp[2][2]
			),
			mvp[3][3] - mvp[3][2]
		};

		auto normalize = [](frustum_plane& p) {
			f32 imag = 1.0f / glm::length(p.normal);
			p.normal *= imag;
			p.distance *= imag;
		};
		normalize(planes[0]);
		normalize(planes[1]);
		normalize(planes[2]);
		normalize(planes[3]);
		normalize(planes[4]);
		normalize(planes[5]);
	}

	bool camera_frustum::contains(const vec3f& point, f32 radius) const {
		f32 nradius = -radius;
		if (glm::dot(planes[0].normal, point) + planes[0].distance <= nradius) return false;
		if (glm::dot(planes[1].normal, point) + planes[1].distance <= nradius) return false;
		if (glm::dot(planes[2].normal, point) + planes[2].distance <= nradius) return false;
		if (glm::dot(planes[3].normal, point) + planes[3].distance <= nradius) return false;
		if (glm::dot(planes[4].normal, point) + planes[4].distance <= nradius) return false;
		if (glm::dot(planes[5].normal, point) + planes[5].distance <= nradius) return false;
		return true;
	}



	camera_component::camera_component() : projection(mat4f(1.0f)), active(false) {
	}

	camera_component::~camera_component() {
	}

	void camera_component::activate() {
		((camera_sys*)system())->activate_camera(entity());
	}

	void camera_component::update_frustum() {
		scene_entity* e = entity();
		mat4f view = mat4f(1.0f);
		if (e->transform) view = e->transform->cascaded_property(&transform_component::transform, &cascade_mat4f);
		frustum.set(projection * view);
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
		} else entity->unbind("remove_camera_component");
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

			entity->bind_interpolatable(component, "projection", &c::projection);
			entity->bind(component, "active", &c::active, true);
			entity->bind(this, "update_frustum", [](entity_system* sys, scene_entity* entity, v8Args args) {
				entity->camera->update_frustum();
			});
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
			entity->unbind("update_frustum");
			entity->unbind("activate");
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
		r2engine::register_entity_property<mat4f>("projection");
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
