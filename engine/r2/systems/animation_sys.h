#pragma once
#include <r2/systems/entity.h>

namespace r2 {
	class keyframe_base {
		public:
			virtual ~keyframe_base() { };
			f32 time;
			void* user_data;
	};

	template <typename T>
	class keyframe : public keyframe_base {
		public:
			keyframe(const T& value, f32 time, void* user_data = nullptr) {
				this->value = value;
				this->time = time;
				this->user_data = user_data;
			}
			virtual ~keyframe() { }

			T value;
	};

	class animation_track_base {
		public:
			virtual ~animation_track_base() { }

			virtual void update(f32 time) = 0;

			mstring name;
			void* user_data;
			mlist<keyframe_base*> keyframes;
			mlist<keyframe_base*>::iterator last_keyframe;
			f32 last_time;
	};

	template <typename T>
	static inline T default_interpolator(const T& a, const T& b, float w) {
		return a + ((b - a) * w);
	}

	template <typename T>
	class animation_track : public animation_track_base {
		public:
			typedef T (*interpolator_callback)(const T&, const T&, float);
			typedef T* (*value_getter)(void*);

			animation_track(const mstring& name, value_getter value, interpolator_callback interpolator, void* user_data = nullptr) {
				this->name = name;
				this->interpolator = interpolator;
				this->user_data = user_data;
				last_keyframe = keyframes.end();
				last_time = 0.0f;
				initial_value = *value(user_data);
				target_value = value;
			}

			virtual ~animation_track() {
				for (auto i = keyframes.begin();i != keyframes.end();i++) {
					keyframe<T>* kf = (keyframe<T>*)*i;
					delete kf;
				}
			}

			inline T get(f32 time) {
				if (!interpolator || keyframes.size() == 0) return initial_value;

				auto start_forward_search = keyframes.begin();
				if (last_keyframe != keyframes.end()) {
					if (time > last_time) {
						// iterate forward normally, starting
						// from last used keyframe
						start_forward_search = last_keyframe;
					} else {
						// iterate backward, starting from
						// last used keyframe

						for (auto i = last_keyframe;i != keyframes.begin();i--) {
							keyframe<T>* f = (keyframe<T>*)*i;
							auto p = std::prev(i);
							keyframe<T>* pf = (keyframe<T>*)*p;
							if (f->time >= time && pf->time <= time) {
								last_keyframe = i;
								last_time = time;
								return interpolator(pf->value, f->value, (time - pf->time) / (f->time - pf->time));
							}
						}

						last_keyframe = keyframes.end();
						last_time = time;
						keyframe<T>* f = (keyframe<T>*)keyframes.front();
						return interpolator(initial_value, f->value, time / f->time);
					}
				}

				if (keyframes.front()->time >= time) {
					last_keyframe = keyframes.end();
					last_time = time;
					keyframe<T>* f = (keyframe<T>*)keyframes.front();
					return interpolator(initial_value, f->value, time / f->time);
				}

				for (auto i = start_forward_search;i != keyframes.end();i++) {
					keyframe<T>* f = (keyframe<T>*)*i;
					auto n = std::next(i);
					if (f->time <= time) {
						if (n == keyframes.end()) {
							last_keyframe = i;
							last_time = time;
							return f->value;
						}
						else {
							keyframe<T>* nf = (keyframe<T>*)*n;
							if (nf->time >= time) {
								last_keyframe = i;
								last_time = time;
								return interpolator(f->value, nf->value, (time - f->time) / (nf->time - f->time));
							}
						}
					}
				}

				// this should never be reached
				last_keyframe = keyframes.end();
				last_time = time;
				return initial_value;
			}

			inline void set(const T& value, float time, void* user_pointer = nullptr) {
				for (auto i = keyframes.begin();i != keyframes.end();i++) {
					keyframe<T>* kf = (keyframe<T>*)*i;
					if (kf->time > time + 0.0001f) {
						keyframes.insert(i, new keyframe<T>(value, time, user_data));
						return;
					} else if (kf->time < time + 0.0001f && kf->time > time - 0.0001f) {
						kf->user_data = user_pointer;
						kf->value = value;
						return;
					}
				}

				keyframes.push_back(new keyframe<T>(value, time, user_data));
			}

			virtual void update(f32 time) {
				T* value = target_value(user_data);
				if (!value) return;
				*value = get(time);
			}

			interpolator_callback interpolator;
			T initial_value;
			value_getter target_value;
	};

	class animation_group {
		public:
			animation_group(const mstring& name, f32 duration, bool loops = false);
			~animation_group();

			template <typename T>
			inline void add_track(const mstring& name, typename animation_track<T>::value_getter value, typename animation_track<T>::interpolator_callback interpolator = default_interpolator<T>, void* user_data = nullptr) {
				animation_track<T>* track = new animation_track<T>(name, value, interpolator, user_data);
				m_tracks[name] = track;
				m_contiguous_tracks.push_back(track);
			}

			void remove_track(const mstring& track);

			template <typename T>
			inline void set(const mstring& track, const T& value, float time, void* user_data = nullptr) {
				auto it = m_tracks.find(track);
				assert(it != m_tracks.end());
				((animation_track<T>*)it->second)->set(value, time, user_data);
			}

			template <typename T>
			inline T get(const mstring& track, float time) {
				auto it = m_tracks.find(name);
				assert(it != m_tracks.end());
				return ((animation_track<T>*)*it)->get(time);
			}

			template <typename T>
			inline animation_track<T>* track(const mstring& track) {
				auto it = m_tracks.find(track);
				assert(it != m_tracks.end());
				return ((KeyframeTrack<T>*)it->second);
			}

			template <typename T>
			inline animation_track<T>* track(size_t idx) {
				return (KeyframeTrack<T>*)m_contiguous_tracks[idx];
			}

			inline size_t track_count() const { return m_contiguous_tracks.size(); }

			animation_track_base* track(const mstring& name);

			animation_track_base* track(size_t idx);

			inline bool playing() const { return m_playing; }

			inline const mstring& name() const { return m_name; }

			inline f32 duration() const { return m_duration; }

			inline bool loops() const { return m_loops; }

			inline f32 current_time() const { return m_time; }

			bool duration(f32 duration);

			void loops(bool loops);

			void update(f32 dt);

			void set_time(f32 time);

			void reset();

			void play();
			
			void pause();

		protected:
			mstring m_name;
			f32 m_duration;
			f32 m_time;
			bool m_loops;
			bool m_playing;

			munordered_map<mstring, animation_track_base*> m_tracks;
			mvector<animation_track_base*> m_contiguous_tracks;
	};

	class animation_component : public scene_entity_component {
		public:
			animation_component();
			~animation_component();

			dynamic_pod_array<animation_group*> animations;
	};

	class animation_sys : public entity_system, periodic_update {
		public:
			~animation_sys();
			static animation_sys* get() {
				if (instance) return instance;
				instance = new animation_sys();
				return instance;
			}

			virtual const size_t component_size() const { return sizeof(animation_component); }

			virtual void initialize();
			virtual void deinitialize();
			virtual void initialize_entity(scene_entity* entity);
			virtual void deinitialize_entity(scene_entity* entity);
			virtual scene_entity_component* create_component(entityId id);
			virtual void bind(scene_entity_component* component, scene_entity* entity);
			virtual void unbind(scene_entity* entity);
			virtual void tick(f32 dt);
			virtual void doUpdate(f32 frameDelta, f32 updateDelta);
			virtual void handle(event* evt);

		protected:
			animation_sys();
			static animation_sys* instance;
	};
};