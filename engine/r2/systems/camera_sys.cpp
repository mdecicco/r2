#include <r2/engine.h>
#include <r2/systems/camera_sys.h>
#include <r2/systems/cascade_functions.h>

namespace r2 {
	camera_frustum::camera_frustum() {
	}

	camera_frustum::~camera_frustum() {
	}

	void camera_frustum::set(const mat4f& vp) {
		planes[pi_right] = {
			vec3f(
				vp[0][3] + vp[0][0],
				vp[1][3] + vp[1][0],
				vp[2][3] + vp[2][0]
			),
			vp[3][3] + vp[3][0]
		};

		planes[pi_left] = {
			vec3f(
				vp[0][3] - vp[0][0],
				vp[1][3] - vp[1][0],
				vp[2][3] - vp[2][0]
			),
			vp[3][3] - vp[3][0]
		};

		planes[pi_bottom] = {
			vec3f(
				vp[0][3] + vp[0][1],
				vp[1][3] + vp[1][1],
				vp[2][3] + vp[2][1]
			),
			vp[3][3] + vp[3][1]
		};

		planes[pi_top] = {
			vec3f(
				vp[0][3] - vp[0][1],
				vp[1][3] - vp[1][1],
				vp[2][3] - vp[2][1]
			),
			vp[3][3] - vp[3][1]
		};

		planes[pi_far] = {
			vec3f(
				vp[0][2],
				vp[1][2],
				vp[2][2]
			),
			vp[3][2]
		};

		planes[pi_near] = {
			vec3f(
				vp[0][3] - vp[0][2],
				vp[1][3] - vp[1][2],
				vp[2][3] - vp[2][2]
			),
			vp[3][3] - vp[3][2]
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



	camera_component::camera_component() {
		m_projection = mat4f(1.0f);
		m_active = false;
		field_of_view = 60.0f;
		vec2f ws = r2engine::get()->window()->get_size();
		width = ws.x;
		height = ws.y;
		near_plane = 0.01f;
		far_plane = 1000.0f;
		orthographic_factor = 0.0f;
	}

	camera_component::~camera_component() {
	}

	void camera_component::activate() {
		((camera_sys*)system())->activate_camera(entity());
	}

	void camera_component::update_projection() {
		if (orthographic_factor == 0.0f) {
			// completely perspective
			m_projection = glm::perspective(glm::radians(field_of_view), width / height, near_plane, far_plane);
		} else if (orthographic_factor == 1.0f) {
			// completely orthographic
			f32 hw = width * 0.5f;
			f32 hh = height * 0.5f;
			m_projection = glm::ortho(-hw, hw, hh, hh, near_plane, far_plane);
		}
		f32 w = width;
		if (w == 0.0f) w = 0.0001f;
		f32 h = height;
		if (h == 0.0f) h = 0.0001f;
		mat4f persp = glm::perspective(glm::radians(field_of_view), w / h, near_plane, far_plane);
		f32 hw = w * 0.5f;
		f32 hh = h * 0.5f;
		mat4f ortho = glm::ortho(-hw, hw, -hh, hh, near_plane, far_plane);
		m_projection = persp + ((ortho - persp) * glm::clamp(orthographic_factor, 0.0f, 1.0f));

		scene_entity* e = entity();
		mat4f view = mat4f(1.0f);
		if (e->transform) view = e->transform->cascaded_property(&transform_component::transform, &cascade_mat4f);
		m_frustum.set(m_projection * view);
	}

	void camera_component::update_frustum() {
		scene_entity* e = entity();
		mat4f view = mat4f(1.0f);
		if (e->transform) view = e->transform->cascaded_property(&transform_component::transform, &cascade_mat4f);
		m_frustum.set(m_projection * view);
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
		} else entity->unbind("camera");
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

			entity->bind_interpolatable(component, "camera", "orthographic_factor", &c::orthographic_factor);
			entity->bind_interpolatable(component, "camera", "field_of_view", &c::field_of_view);
			entity->bind_interpolatable(component, "camera", "width", &c::width);
			entity->bind_interpolatable(component, "camera", "height", &c::height);
			entity->bind_interpolatable(component, "camera", "near_plane", &c::near_plane);
			entity->bind_interpolatable(component, "camera", "far_plane", &c::far_plane);
			entity->bind(component, "camera", "projection", &c::m_projection, true);

			entity->bind(this, "camera", "is_active", [](entity_system* sys, scene_entity* entity, v8Args args) {
				args.GetReturnValue().Set(v8::Boolean::New(args.GetIsolate(), entity->camera->is_active()));
			});
			entity->bind(this, "camera", "is_orthographic", [](entity_system* sys, scene_entity* entity, v8Args args) {
				args.GetReturnValue().Set(v8::Boolean::New(args.GetIsolate(), entity->camera->is_orthographic()));
			});
			entity->bind(this, "camera", "update_projection", [](entity_system* sys, scene_entity* entity, v8Args args) {
				entity->camera->update_projection();
			});
			entity->bind(this, "camera", "activate", [](entity_system* sys, scene_entity* entity, v8Args args) {
				((camera_sys*)sys)->activate_camera(entity);
			});
			entity->bind(this, "camera", "remove", [](entity_system* system, scene_entity* entity, v8Args args) {
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
		r2engine::register_entity_property<f32>("camera.orthographic_factor");
		r2engine::register_entity_property<f32>("camera.field_of_view");
		r2engine::register_entity_property<f32>("camera.width");
		r2engine::register_entity_property<f32>("camera.height");
		r2engine::register_entity_property<f32>("camera.near_plane");
		r2engine::register_entity_property<f32>("camera.far_plane");
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
		if (cam->m_active) {
			state.disable();
			return;
		}

		// set the active camera to inactive, if there is one
		state->for_each<camera_component>([](camera_component* c) {
			bool should_break = c->m_active;

			// deactivate
			c->m_active = false;

			// break if the active camera is found
			return !should_break;
		});

		// activate this one
		cam->m_active = true;
		curScene->camera = entity;
		state.disable();
	}
};
