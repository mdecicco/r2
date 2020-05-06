#include <r2/systems/animation_sys.h>
#include <r2/engine.h>

namespace r2 {
	animation_group::animation_group(const mstring& name, f32 duration, bool loops) {
		m_name = name;
		m_duration = duration;
		m_time = 0.0f;
		m_loops = loops;
		m_playing = false;
	}

	animation_group::~animation_group() {
		for (auto i = m_contiguous_tracks.begin();i != m_contiguous_tracks.end();i++) {
			delete *i;
		}
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

	void animation_group::update(f32 dt) {
		if (!m_playing) return;

		m_time += dt;
		bool reached_end = false;
		if (m_time > m_duration) {
			reached_end = true;
			m_time = m_duration;
		}

		for (auto i = m_contiguous_tracks.begin();i != m_contiguous_tracks.end();i++) {
			(*i)->update(m_time);
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


	animation_component::animation_component() {
	}

	animation_component::~animation_component() {
	}


	animation_sys* animation_sys::instance = nullptr;
	animation_sys::animation_sys() {
	}

	animation_sys::~animation_sys() {
	}

	void animation_sys::initialize_entity(scene_entity* entity) {
		if (!entity->is_scripted()) return;
		entity->bind(this, "add_animation_component", [](entity_system* system, scene_entity* entity, v8Args args) {
			system->addComponentTo(entity);
		});
	}
	void animation_sys::deinitialize_entity(scene_entity* entity) {
		if (!entity->is_scripted()) return;
		auto s = state();
		s.enable();
		if (!s->contains_entity(entity->id())) entity->unbind("animation");
		else entity->unbind("add_animation_component");
		s.disable();
	}

	scene_entity_component* animation_sys::create_component(entityId id) {
		auto s = state();
		s.enable();
		auto out = s->create<animation_component>(id);
		s.disable();
		return out;
	}

	void animation_sys::bind(scene_entity_component* component, scene_entity* entity) {
		using c = animation_component;
		if (entity->is_scripted()) {
			entity->unbind("add_animation_component");
			entity->bind(this, "animation", "remove", [](entity_system* system, scene_entity* entity, v8Args args) {
				system->removeComponentFrom(entity);
			});
		}
		entity->animation = component_ref<c*>(this, component->id());
	}
	void animation_sys::unbind(scene_entity* entity) {
		if (entity->is_scripted()) {
			entity->bind(this, "add_animation_component", [](entity_system* system, scene_entity* entity, v8Args args) {
				system->addComponentTo(entity);
			});
		}
		entity->animation.clear();
	}

	void animation_sys::initialize() {
		initialize_periodic_update();
		setUpdateFrequency(60.0f);
		start_periodic_updates();
	}

	void animation_sys::deinitialize() {
		destroy_periodic_update();
	}

	void animation_sys::tick(f32 dt) {
		update(dt);
	}

	void animation_sys::doUpdate(f32 frameDelta, f32 updateDelta) {
		auto& state = this->state();
		state.enable();
		state->for_each<animation_component>([updateDelta](animation_component* comp) {
			comp->animations.for_each([updateDelta](animation_group** anim) {
				(*anim)->update(updateDelta);
				return true;
			});

			return true;
		});
		state.disable();
	}

	void animation_sys::handle(event* evt) {
	}
};
