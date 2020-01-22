#include <r2/utilities/fly_camera.h>
#include <r2/engine.h>

namespace r2 {
	fly_camera_entity::fly_camera_entity(const mstring& name) : scene_entity(name) {
		time = 0.0f;
		deadzone = 0.2f;
		j_pos = vec3f(0, 0, 0);
		j_rot = mat4f(1.0f);
	}

	void fly_camera_entity::onInitialize() {
		setUpdateFrequency(60.0f);
		transform_sys::get()->addComponentTo(this);
		camera_sys::get()->addComponentTo(this);
		camera->activate();
		if (r2engine::input()->joystick_count() > 0) {
			transform->transform = glm::translate(j_rot, j_pos);
		}

		vec2f screen = r2engine::get()->window()->get_size();
		camera->projection = glm::perspective(glm::radians(90.0f), screen.x / screen.y, 0.1f, 200.0f);

		moveSpeed = 25.0f;
	}

	void fly_camera_entity::onUpdate(f32 frameDt, f32 updateDt) {
		time += updateDt;
		vec2f screen = r2engine::get()->window()->get_size();
		camera->projection = glm::perspective(glm::radians(90.0f), screen.x / screen.y, 0.1f, 1000.0f);

		if (r2engine::input()->joystick_count() > 0) {
			auto js = r2engine::input()->joystick(0);
			auto jState = js->getJoyStickState();
			if (jState.mAxes[4].abs > deadzone) moveSpeed += (f32(jState.mAxes[4].abs) / 32767.0f) * 10.0f * updateDt;
			if (jState.mAxes[5].abs > deadzone) moveSpeed -= (f32(jState.mAxes[5].abs) / 32767.0f) * 10.0f * updateDt;

			lstick = vec2f(f32(jState.mAxes[1].abs) / 32767.0f, f32(jState.mAxes[0].abs) / 32767.0f);
			rstick = vec2f(f32(jState.mAxes[3].abs) / 32767.0f, f32(jState.mAxes[2].abs) / 32767.0f);

			bool changed = false;

			if (glm::length(lstick) > deadzone) {
				vec3f translate = vec4f(-lstick.x, 0, -lstick.y, 1.0f) * j_rot;
				translate *= moveSpeed * updateDt;
				j_pos += translate;
				changed = true;
			}

			if (glm::length(rstick) > deadzone) {
				if (rstick.x > deadzone || rstick.x < -deadzone) {
					vec3f axis = vec4f(0, 1, 0, 1);
					j_rot *= glm::rotate(mat4f(1.0f), rstick.x * 2.0f * updateDt, axis);
				}
				if (rstick.y > deadzone || rstick.y < -deadzone) {
					vec3f axis = vec4f(1, 0, 0, 1) * j_rot;
					j_rot *= glm::rotate(mat4f(1.0f), rstick.y * 2.0f * updateDt, axis);
				}
				changed = true;
			}

			if (changed) transform->transform = glm::translate(j_rot, j_pos);
		}
	}
};