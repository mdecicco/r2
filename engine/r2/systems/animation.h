#pragma once
#include <r2/managers/memman.h>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

namespace r2 {
	class scene_entity;
	class data_container;

	namespace interpolate {
		enum interpolation_transition_mode;
		typedef f32 (*InterpolationFactorCallback)(f32);
		InterpolationFactorCallback from_enum(interpolation_transition_mode mode);
	};

	class keyframe_base {
		public:
			virtual ~keyframe_base() { };
			virtual inline void* data() = 0;
			f32 time;
			void* user_data;
			interpolate::InterpolationFactorCallback interpolation_factor_cb;
			interpolate::interpolation_transition_mode interpolation_mode;
	};

	template <typename T>
	class keyframe : public keyframe_base {
		public:
			keyframe(const T& value, f32 time, interpolate::interpolation_transition_mode mode, void* user_data = nullptr) {
				this->value = value;
				this->time = time;
				this->user_data = user_data;
				interpolation_factor_cb = interpolate::from_enum(mode);
				interpolation_mode = mode;
			}
			virtual ~keyframe() { }

			virtual void* data() { return &value; }

			T value;
	};

	class animation_track_base {
		public:
			virtual ~animation_track_base() { }

			virtual inline void* initial_value_data() = 0;
			virtual inline size_t value_size() const = 0;

			virtual void update(f32 time, scene_entity* target) = 0;

			mstring name;
			void* user_data;
			mlist<keyframe_base*> keyframes;
			mlist<keyframe_base*>::iterator last_keyframe;
			f32 last_time;
	};

	template <typename T>
	static inline typename std::enable_if<!std::is_same<T, mat4f>::value, T>::type default_interpolator(const T& a, const T& b, f32 w) {
		return a + ((b - a) * w);
	}

	template <typename T>
	static inline typename std::enable_if<std::is_same<T, mat4f>::value, mat4f>::type default_interpolator(const T& a, const T& b, f32 w) {
		vec3f t_a, s_a, sk_a;
		vec4f p_a;
		glm::quat r_a;
		glm::decompose(a, s_a, r_a, t_a, sk_a, p_a);

		vec3f t_b, s_b, sk_b;
		vec4f p_b;
		glm::quat r_b;
		glm::decompose(b, s_b, r_b, t_b, sk_b, p_b);

		
		vec3f t_r = t_a + ((t_b - t_a) * w);
		vec3f s_r = s_a + ((s_b - s_a) * w);
		glm::quat r_r = glm::slerp(r_a, r_b, w);

		mat4f o = mat4f(1.0f);
		o = glm::translate(o, t_r);
		o = glm::scale(o, s_r);
		o *= glm::toMat4(r_r);
		return o;
	}

	template <typename T>
	class animation_track : public animation_track_base {
		public:
			typedef T (*interpolator_callback)(const T&, const T&, float);
			typedef void (*value_setter)(const T&, scene_entity*, void*);

			animation_track(const mstring& name, const T& initial_value, value_setter set, interpolator_callback interpolator, void* user_data = nullptr) {
				this->name = name;
				this->interpolator = interpolator;
				this->user_data = user_data;
				this->initial_value = initial_value;
				last_keyframe = keyframes.end();
				last_time = 0.0f;
				set_value = set;
			}

			virtual ~animation_track() {
				for (auto i = keyframes.begin();i != keyframes.end();i++) {
					keyframe<T>* kf = (keyframe<T>*)*i;
					delete kf;
				}
			}

			virtual inline void* initial_value_data() { return &initial_value; }

			virtual inline size_t value_size() const { return sizeof(T); }

			inline T get(f32 time) {
				if (!interpolator || keyframes.size() == 0) return initial_value;

				auto start_forward_search = keyframes.begin();
				if (last_keyframe != keyframes.end()) {
					if (time > (*last_keyframe)->time) {
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
								return interpolator(pf->value, f->value, f->interpolation_factor_cb((time - pf->time) / (f->time - pf->time)));
							}
						}

						last_keyframe = keyframes.end();
						last_time = time;
						keyframe<T>* f = (keyframe<T>*)keyframes.front();
						if (f->time == 0.0f) {
							auto n = std::next(keyframes.begin());
							if (n != keyframes.end()) {
								return interpolator(f->value, ((keyframe<T>*)*n)->value, (*n)->interpolation_factor_cb(time / (*n)->time));
							} else return f->value;
						}
						return interpolator(initial_value, f->value, f->interpolation_factor_cb(time / f->time));
					}
				}

				if (keyframes.front()->time >= time) {
					last_keyframe = keyframes.end();
					last_time = time;
					keyframe<T>* f = (keyframe<T>*)keyframes.front();
					return interpolator(initial_value, f->value, f->interpolation_factor_cb(time / f->time));
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
								return interpolator(f->value, nf->value, nf->interpolation_factor_cb((time - f->time) / (nf->time - f->time)));
							}
						}
					}
				}

				// this should never be reached
				last_keyframe = keyframes.end();
				last_time = time;
				return initial_value;
			}

			inline keyframe_base* set(const T& value, float time, interpolate::interpolation_transition_mode mode, void* user_pointer = nullptr) {
				for (auto i = keyframes.begin();i != keyframes.end();i++) {
					keyframe<T>* kf = (keyframe<T>*)*i;
					if (kf->time > time + 0.0001f) {
						keyframe<T>* f = new keyframe<T>(value, time, mode, user_data);
						keyframes.insert(i, f);
						return f;
					} else if (kf->time < time + 0.0001f && kf->time > time - 0.0001f) {
						kf->user_data = user_pointer;
						kf->value = value;
						kf->interpolation_factor_cb = interpolate::from_enum(mode);
						kf->interpolation_mode = mode;
						return kf;
					}
				}

				keyframe<T>* f = new keyframe<T>(value, time, mode, user_data);
				keyframes.push_back(f);
				return f;
			}

			virtual void update(f32 time, scene_entity* target) {
				set_value(get(time), target, user_data);
			}

			interpolator_callback interpolator;
			T initial_value;
			value_setter set_value;
	};

	class animation_deserializer {
		public:
			typedef keyframe_base* (*create_keyframe_func)(const mstring& /*track_name*/, f32 /*time*/, void* /*value_data*/, animation_track_base* /*dest_track*/, interpolate::interpolation_transition_mode /*transition_mode*/);
			typedef animation_track_base* (*create_track_func)(const mstring& /*track_name*/, void* /*initial_value_data*/);

			void register_track(const mstring& track, create_keyframe_func create_keyframe, create_track_func create_track);

			struct track_info {
				mstring propertyName;
				create_keyframe_func keyframe_func;
				create_track_func track_func;
			};

			munordered_map<mstring, track_info> track_properties;
	};

	class animation_group {
		public:
			animation_group(const mstring& name, f32 duration, bool loops = false);
			animation_group(data_container* in, scene_entity* entity);
			animation_group(data_container* in, const animation_deserializer& deserializer);
			~animation_group();

			bool serialize(data_container* out);
			bool deserialize(data_container* in, scene_entity* entity);
			bool deserialize(data_container* in, const animation_deserializer& deserializer);

			template <typename T>
			inline animation_track_base* add_track(const mstring& name, const T& initial_value, typename animation_track<T>::value_setter set_value, typename animation_track<T>::interpolator_callback interpolator = default_interpolator<T>, void* user_data = nullptr) {
				animation_track<T>* track = new animation_track<T>(name, initial_value, set_value, interpolator, user_data);
				m_tracks[name] = track;
				m_contiguous_tracks.push_back(track);
				return track;
			}

			void remove_track(const mstring& track);

			template <typename T>
			inline void set(const mstring& track, const T& value, float time, interpolate::interpolation_transition_mode mode, void* user_data = nullptr) {
				auto it = m_tracks.find(track);
				if (it == m_tracks.end()) return;
				((animation_track<T>*)it->second)->set(value, time, mode, user_data);
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
				if (it == m_tracks.end()) return nullptr;
				return ((animation_track<T>*)it->second);
			}

			template <typename T>
			inline animation_track<T>* track(size_t idx) {
				return (animation_track<T>*)m_contiguous_tracks[idx];
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

			void update(f32 dt, scene_entity* target);

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
};