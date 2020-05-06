#pragma once
#include <r2/systems/entity.h>

namespace r2 {
	class keyframe_base {
		public:
			virtual ~keyframe_base() = 0;
			f32 time;
			void* user_data;
	};

	template <typename T>
	class keyframe {
		public:
			keyframe(const T& value, f32 time, void* user_data = nullptr) {
				this->value = value;
				this->time = time;
				this->user_data = user_data;
			}

			T value;
	};

	class animation_track_base {
		public:
			virtual ~animation_track_base() = 0;
			mstring name;
			bool loops;
			void* user_data;
			mlist<keyframe_base*> keyframes;
	};

	template <typename T>
	static inline T default_interpolator(const T& a, const T& b, float w) {
		return a + ((b - a) * w);
	}

	template <typename T>
	class animation_track : public animation_track_base {
		public:
			typedef T (*interpolator_callback)(const T&, const T&, float);

			animation_track(const mstring& name, T* value, bool loops, interpolator_callback interpolator, void* user_data = nullptr) {
				this->name = name;
				this->loops = loops;
				this->interpolator = interpolator;
				this->user_data = user_data;
				initial_value = *value;
			}

			virtual ~animation_track() {
			}


			interpolator_callback interpolator;
			T initial_value;
			T* target_value;
	};

	class animation_component : public scene_entity_component {
		public:
			animation_component();
			~animation_component();
	};

	class animation_sys : public entity_system {
		public:
			~animation_sys();
			static animation_sys* get() {
				if (instance) return instance;
				instance = new animation_sys();
				return instance;
			}

			virtual const size_t component_size() const { return sizeof(animation_component); }

			virtual void initialize();
			virtual void initialize_entity(scene_entity* entity);
			virtual void deinitialize_entity(scene_entity* entity);
			virtual scene_entity_component* create_component(entityId id);
			virtual void bind(scene_entity_component* component, scene_entity* entity);
			virtual void unbind(scene_entity* entity);
			virtual void tick(f32 dt);
			virtual void handle(event* evt);

		protected:
			animation_sys();
			static animation_sys* instance;
	};
};