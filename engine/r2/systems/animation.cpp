#include <r2/systems/animation.h>
#include <r2/engine.h>

namespace r2 {
	void animation_deserializer::register_track(const mstring& track, create_keyframe_func create_keyframe, create_track_func create_track) {
		auto it = track_properties.find(track);
		if (it != track_properties.end()) {
			r2Error("Track '%s' has already been registered", track.c_str());
			return;
		}

		track_properties[track] = {
			track,
			create_keyframe,
			create_track
		};
	}

	animation_group::animation_group(const mstring& name, f32 duration, bool loops) {
		m_name = name;
		m_duration = duration;
		m_time = 0.0f;
		m_loops = loops;
		m_playing = false;
	}

	animation_group::animation_group(data_container* in, scene_entity* entity) {
		if (!deserialize(in, entity)) {
			throw std::exception("Failed to deserialize exception!");
		}
	}

	animation_group::animation_group(data_container* in, const animation_deserializer& deserialzer) {
		if (!deserialize(in, deserialzer)) {
			throw std::exception("Failed to deserialize exception!");
		}
	}

	animation_group::~animation_group() {
		for (auto i = m_contiguous_tracks.begin();i != m_contiguous_tracks.end();i++) delete *i;
	}

	bool animation_group::serialize(data_container* out) {
		u8 name_len = m_name.length();
		if (!out->write(name_len)) return false;
		if (!out->write_data(m_name.data(), name_len)) return false;

		if (!out->write(m_loops)) return false;
		if (!out->write(m_duration)) return false;

		u8 track_count = m_contiguous_tracks.size();
		if (!out->write(track_count)) return false;

		for (u32 t = 0;t < m_contiguous_tracks.size();t++) {
			animation_track_base* track = m_contiguous_tracks[t];
			name_len = track->name.length();
			if (!out->write(name_len)) return false;
			if (!out->write_data(track->name.data(), name_len)) return false;

			u8 keyframe_value_size = track->value_size();
			if (!out->write(keyframe_value_size)) return false;

			if (!out->write_data(track->initial_value_data(), keyframe_value_size)) return false;

			u16 keyframe_count = track->keyframes.size();
			if (!out->write(keyframe_count)) return false;

			for (auto it = track->keyframes.begin();it != track->keyframes.end();it++) {
				keyframe_base* k = *it;
				if (!out->write(k->time)) return false;
				if (!out->write(k->interpolation_mode)) return false;
				if (!out->write_data(k->data(), keyframe_value_size)) return false;
			}
		}

		return true;
	}

	bool animation_group::deserialize(data_container* in, scene_entity* entity) {
		u8 name_len = 0;
		if (!in->read(name_len)) return false;

		char* name = new char[name_len];
		if (!in->read_data(name, name_len)) return false;
		m_name = mstring(name, name_len);
		delete [] name;

		if (!in->read(m_loops)) return false;
		if (!in->read(m_duration)) return false;

		u8 track_count = 0;
		if (!in->read(track_count)) return false;

		for (u8 t = 0;t < track_count;t++) {
			name_len = 0;
			if (!in->read(name_len)) {
				for (auto i = m_contiguous_tracks.begin();i != m_contiguous_tracks.end();i++) delete *i;
				m_contiguous_tracks.clear();
				m_tracks.clear();
				return false;
			}

			name = new char[name_len];
			if (!in->read_data(name, name_len)) {
				for (auto i = m_contiguous_tracks.begin();i != m_contiguous_tracks.end();i++) delete *i;
				m_contiguous_tracks.clear();
				m_tracks.clear();
				return false;
			}
			mstring track_name = mstring(name, name_len);
			delete [] name;

			animation_track_base* track = entity->animate_prop(track_name, this);
			if (!track) {
				for (auto i = m_contiguous_tracks.begin();i != m_contiguous_tracks.end();i++) delete *i;
				m_contiguous_tracks.clear();
				m_tracks.clear();
				return false;
			}

			u8 value_size = 0;
			if (!in->read(value_size)) {
				for (auto i = m_contiguous_tracks.begin();i != m_contiguous_tracks.end();i++) delete *i;
				m_contiguous_tracks.clear();
				m_tracks.clear();
				return false;
			}

			if (!r2engine::deserialize_entity_property(track_name, track->initial_value_data(), in)) {
				for (auto i = m_contiguous_tracks.begin();i != m_contiguous_tracks.end();i++) delete *i;
				m_contiguous_tracks.clear();
				m_tracks.clear();
				return false;
			}

			u16 keyframe_count = 0;
			if (!in->read(keyframe_count)) {
				for (auto i = m_contiguous_tracks.begin();i != m_contiguous_tracks.end();i++) delete *i;
				m_contiguous_tracks.clear();
				m_tracks.clear();
				return false;
			}

			for (u16 kn = 0;kn < keyframe_count;kn++) {
				if (!in->read(m_time)) {
					m_time = 0.0f;
					for (auto i = m_contiguous_tracks.begin();i != m_contiguous_tracks.end();i++) delete *i;
					m_contiguous_tracks.clear();
					m_tracks.clear();
					return false;
				}
				
				interpolate::interpolation_transition_mode mode;
				if (!in->read(mode)) {
					m_time = 0.0f;
					for (auto i = m_contiguous_tracks.begin();i != m_contiguous_tracks.end();i++) delete *i;
					m_contiguous_tracks.clear();
					m_tracks.clear();
					return false;
				}
				keyframe_base* k = entity->create_keyframe(track_name, this, mode);
				m_time = 0.0f;
				if (!k) {
					for (auto i = m_contiguous_tracks.begin();i != m_contiguous_tracks.end();i++) delete *i;
					m_contiguous_tracks.clear();
					m_tracks.clear();
					return false;
				}

				if (!r2engine::deserialize_entity_property(track_name, k->data(), in)) {
					for (auto i = m_contiguous_tracks.begin();i != m_contiguous_tracks.end();i++) delete *i;
					m_contiguous_tracks.clear();
					m_tracks.clear();
					return false;
				}
			}
		}

		return true;
	}

	bool animation_group::deserialize(data_container* in, const animation_deserializer& deserializer) {
		u8 name_len = 0;
		if (!in->read(name_len)) return false;

		char* name = new char[name_len];
		if (!in->read_data(name, name_len)) return false;
		m_name = mstring(name, name_len);
		delete [] name;

		if (!in->read(m_loops)) return false;
		if (!in->read(m_duration)) return false;

		u8 track_count = 0;
		if (!in->read(track_count)) return false;

		for (u8 t = 0;t < track_count;t++) {
			name_len = 0;
			if (!in->read(name_len)) {
				for (auto i = m_contiguous_tracks.begin();i != m_contiguous_tracks.end();i++) delete *i;
				m_contiguous_tracks.clear();
				m_tracks.clear();
				return false;
			}

			name = new char[name_len];
			if (!in->read_data(name, name_len)) {
				for (auto i = m_contiguous_tracks.begin();i != m_contiguous_tracks.end();i++) delete *i;
				m_contiguous_tracks.clear();
				m_tracks.clear();
				return false;
			}
			mstring track_name = mstring(name, name_len);
			delete [] name;

			auto ti_it = deserializer.track_properties.find(track_name);
			if (ti_it == deserializer.track_properties.end()) {
				for (auto i = m_contiguous_tracks.begin();i != m_contiguous_tracks.end();i++) delete *i;
				m_contiguous_tracks.clear();
				m_tracks.clear();
				return false;
			}
			const animation_deserializer::track_info& ti = ti_it->second;

			u8 value_size = 0;
			if (!in->read(value_size)) {
				for (auto i = m_contiguous_tracks.begin();i != m_contiguous_tracks.end();i++) delete *i;
				m_contiguous_tracks.clear();
				m_tracks.clear();
				return false;
			}

			u8* initial_value = new u8[value_size];
			if (!in->read_data(initial_value, value_size)) {
				for (auto i = m_contiguous_tracks.begin();i != m_contiguous_tracks.end();i++) delete *i;
				m_contiguous_tracks.clear();
				m_tracks.clear();
				return false;
			}

			animation_track_base* track = ti.track_func(track_name, initial_value);
			delete [] initial_value;
			if (!track) {
				for (auto i = m_contiguous_tracks.begin();i != m_contiguous_tracks.end();i++) delete *i;
				m_contiguous_tracks.clear();
				m_tracks.clear();
				return false;
			}

			u16 keyframe_count = 0;
			if (!in->read(keyframe_count)) {
				for (auto i = m_contiguous_tracks.begin();i != m_contiguous_tracks.end();i++) delete *i;
				m_contiguous_tracks.clear();
				m_tracks.clear();
				return false;
			}

			for (u16 kn = 0;kn < keyframe_count;kn++) {
				f32 time = 0.0f;
				if (!in->read(time)) {
					for (auto i = m_contiguous_tracks.begin();i != m_contiguous_tracks.end();i++) delete *i;
					m_contiguous_tracks.clear();
					m_tracks.clear();
					return false;
				}

				interpolate::interpolation_transition_mode mode;
				if (!in->read(mode)) {
					for (auto i = m_contiguous_tracks.begin();i != m_contiguous_tracks.end();i++) delete *i;
					m_contiguous_tracks.clear();
					m_tracks.clear();
					return false;
				}

				u8* value = new u8[value_size];
				if (!in->read_data(value, value_size)) {
					for (auto i = m_contiguous_tracks.begin();i != m_contiguous_tracks.end();i++) delete *i;
					m_contiguous_tracks.clear();
					m_tracks.clear();
					return false;
				}

				keyframe_base* k = ti.keyframe_func(track_name, time, value, track, mode);
				delete [] value;
				if (!k) {
					for (auto i = m_contiguous_tracks.begin();i != m_contiguous_tracks.end();i++) delete *i;
					m_contiguous_tracks.clear();
					m_tracks.clear();
					return false;
				}
			}
		}

		return true;
	}


	void animation_group::remove_track(const mstring& track) {
		auto i = m_tracks.find(track);
		if (i == m_tracks.end()) return;

		for (auto vi = m_contiguous_tracks.begin();vi != m_contiguous_tracks.end();vi++) {
			if (i->second == *vi) {
				m_contiguous_tracks.erase(vi);
				break;
			}
		}

		delete i->second;
		m_tracks.erase(i);
	}

	animation_track_base* animation_group::track(const mstring& track) {
		auto i = m_tracks.find(track);
		if (i == m_tracks.end()) return nullptr;
		return i->second;
	}

	animation_track_base* animation_group::track(size_t idx) {
		return m_contiguous_tracks[idx];
	}

	bool animation_group::duration(f32 duration) {
		if (duration >= m_duration) {
			m_duration = duration;
			return true;
		}

		for (auto i = m_contiguous_tracks.begin();i != m_contiguous_tracks.end();i++) {
			animation_track_base* track = *i;
			if (track->keyframes.size() > 0 && track->keyframes.back()->time >= duration - 0.0001f) {
				r2Warn("Can't decrease animation '%s' duration to %.2f, track '%s' contains keyframes beyond that time", m_name.c_str(), duration, track->name.c_str());
				return false;
			}
		}

		m_duration = duration;
		return true;
	}

	void animation_group::loops(bool loops) {
		m_loops = loops;
	}

	void animation_group::update(f32 dt, scene_entity* target) {
		if (!m_playing) return;

		m_time += dt;
		bool reached_end = false;
		if (m_time > m_duration) {
			reached_end = true;
			m_time = m_duration;
		}

		for (auto i = m_contiguous_tracks.begin();i != m_contiguous_tracks.end();i++) {
			(*i)->update(m_time, target);
		}

		if (reached_end) {
			m_time = 0.0f;
			if (!m_loops) m_playing = false;
		}
	}

	void animation_group::set_time(f32 time) {
		if (time > m_duration) m_time = m_duration;
		else if (time < 0.0f) m_time = 0.0f;
		else m_time = time;
	}

	void animation_group::reset() {
		m_playing = false;
		m_time = 0.0f;
	}

	void animation_group::play() {
		m_playing = true;
	}

	void animation_group::pause() {
		m_playing = false;
	}
};