#pragma once
#include <r2/systems/entity.h>

namespace r2 {
	class fly_camera_entity : public scene_entity {
		public:
			fly_camera_entity(const mstring& name = "fly_camera");

			~fly_camera_entity() { }

			virtual void onInitialize();

			virtual void onUpdate(f32 frameDt, f32 updateDt);

			virtual void belowFrequencyWarning(f32 percentLessThanDesired, f32 desiredFreq, f32 timeSpentLowerThanDesired) { }

			f32 time;
			vec2f lstick;
			vec2f rstick;
			f32 deadzone;

			vec3f j_pos;
			mat4f j_rot;
			f32 moveSpeed;
	};
};