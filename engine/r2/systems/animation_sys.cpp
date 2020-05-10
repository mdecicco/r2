#include <r2/systems/animation_sys.h>
#include <r2/engine.h>

namespace r2 {
	animation_component::animation_component() {
	}

	animation_component::~animation_component() {
	}



	animation_sync::animation_sync() {
		m_loops = false;
		m_playing = false;
		m_duration = 0.0f;
		m_time = 0.0f;
	}

	animation_sync::~animation_sync() {
	}

	void animation_sync::add(animation_group* anim) {
		if (anim->m_sync) return;
		anim->m_sync = this;

		m_anims.push_back(anim);
		if (anim->duration() > m_duration) m_duration = anim->duration();
	}

	void animation_sync::remove(animation_group* anim) {
		if (anim->m_sync != this) return;

		f32 max_dur = 0.0f;
		for (auto it = m_anims.begin();it != m_anims.end();it++) {
			if ((*it) == anim) {
				auto n = std::next(it);
				m_anims.erase(it);
				anim->m_sync = nullptr;
				it = n;
			}

			if (it == m_anims.end()) break;
			if ((*it)->duration() > max_dur) max_dur = (*it)->duration();
		}

		m_duration = max_dur;
	}

	void animation_sync::update_duration() {
		m_duration = 0.0f;
		for (auto it = m_anims.begin();it != m_anims.end();it++) {
			animation_group* g = *it;
			if (g->duration() > m_duration) m_duration = g->duration();
		}
	}

	void animation_sync::update(f32 dt) {
		if (m_willPauseNextFrame) {
			m_playing = false;
			m_time = 0.0f;
			return;
		}
		if (!m_playing) return;

		m_time += dt;
		if (m_time > m_duration) {
			m_time = m_duration;
			m_willPauseNextFrame = !m_loops;
		}
	}

	void animation_sync::set_time(f32 time) {
		if (time > m_duration) m_time = m_duration;
		else if (time < 0.0f) m_time = 0.0f;
		else m_time = time;
	}

	void animation_sync::play() {
		m_playing = true;
		m_willPauseNextFrame = false;
	}

	void animation_sync::pause() {
		m_playing = false;
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
		for (auto s = m_syncs.begin();s != m_syncs.end();s++) {
			(*s)->update(updateDelta);
		}
		
		auto& state = this->state();
		state.enable();
		state->for_each<animation_component>([updateDelta](animation_component* comp) {
			scene_entity* target = comp->entity();
			comp->animations.for_each([target, updateDelta](animation_group** anim) {
				(*anim)->update(updateDelta, target);
				return true;
			});

			return true;
		});
		state.disable();
	}

	void animation_sys::handle(event* evt) {
	}

	void animation_sys::add_sync(animation_sync* sync) {
		for (auto s = instance->m_syncs.begin();s != instance->m_syncs.end();s++) {
			if ((*s) == sync) return;
		}
		instance->m_syncs.push_back(sync);
	}

	void animation_sys::remove_sync(animation_sync* sync) {
		for (auto s = instance->m_syncs.begin();s != instance->m_syncs.end();s++) {
			if ((*s) == sync) {
				instance->m_syncs.erase(s);
				return;
			}
		}
	}
};
