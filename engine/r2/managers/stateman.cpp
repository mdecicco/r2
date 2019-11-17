#include <r2/engine.h>

#include <v8pp/class.hpp>
#include <v8pp/utility.hpp>

using namespace v8;
using namespace v8pp;
#define v8str(str) String::NewFromUtf8(isolate, str)

namespace r2 {
    state::state(v8Args args) :
		m_mgr(r2engine::get()->states()),
		m_updateFrequency(0.0f),
		isolate(args.GetIsolate()),
		m_averageUpdateDuration(16) 
	{
		auto global = isolate->GetCurrentContext()->Global();

		if (args.Length() != 1) {
			r2Error("No name passed to state");
			destroy();
			return;
		}

		if (!args[0]->IsString()) {
			r2Error("State constructor accepts one parameter, and it's the name of the state");
			destroy();
			return;
		}

		string name = convert<string>::from_v8(isolate, args[0]);
		if (m_mgr->get_state(name)) {
			r2Error("State \"%s\" has already been registered.", name.c_str());
			destroy();
			return;
		}
		m_name = name;

		LocalObjectHandle prototype = LocalObjectHandle::Cast(args.This()->GetPrototype());

		Local<Array> instanceProps = args.This()->GetOwnPropertyNames(isolate->GetCurrentContext()).ToLocalChecked();
		for(int i = 0;i < instanceProps->Length();i++) {
			LocalValueHandle v = instanceProps->Get(i);
			string s = r2::var(isolate, v);
			printf("instanceProps: %s\n", s.c_str());
		}

		Local<Array> prototypeProps = prototype->GetOwnPropertyNames(isolate->GetCurrentContext()).ToLocalChecked();
		for(int i = 0;i < prototypeProps->Length();i++) {
			LocalValueHandle v = prototypeProps->Get(i);
			string s = r2::var(isolate, v);
			printf("prototypeProps: %s\n", s.c_str());
		}
    }

    state::~state() {
		r2engine* e = r2engine::get();
		if (e->states()->current() == this) {
			r2Warn("State \"%s\" is being unregistered automatically as a result of the deletion of the state", m_name.c_str());
			e->states()->unregister_state(this);
		}
		destroy();
    }

	void state::init() {
		LocalObjectHandle self = convert<state*>::to_v8(isolate, this);

		LocalValueHandle value = self->Get(v8str("willBecomeActive"));
		if (!value->IsUndefined() && !value->IsFunction()) {
			r2Error("State has a global \"state\" variable with a \"willBecomeActive\" property that is not a function.");
			destroy();
			return;
		} else if (!value->IsUndefined()) m_willBecomeActive.Reset(isolate, LocalFunctionHandle::Cast(value));

		value = self->Get(v8str("becameActive"));
		if (!value->IsUndefined() && !value->IsFunction()) {
			r2Error("State has a global \"state\" variable with a \"becameActive\" property that is not a function.");
			destroy();
			return;
		} else if (!value->IsUndefined()) m_becameActive.Reset(isolate, LocalFunctionHandle::Cast(value));

		value = self->Get(v8str("willBecomeInactive"));
		if (!value->IsUndefined() && !value->IsFunction()) {
			r2Error("State has a global \"state\" variable with a \"willBecomeInactive\" property that is not a function.");
			destroy();
			return;
		} else if (!value->IsUndefined()) m_willBecomeInactive.Reset(isolate, LocalFunctionHandle::Cast(value));

		value = self->Get(v8str("becameInactive"));
		if (!value->IsUndefined() && !value->IsFunction()) {
			r2Error("State has a global \"state\" variable with a \"becameInactive\" property that is not a function.");
			destroy();
			return;
		} else if (!value->IsUndefined()) m_becameInactive.Reset(isolate, LocalFunctionHandle::Cast(value));

		value = self->Get(v8str("willBeDestroyed"));
		if (!value->IsUndefined() && !value->IsFunction()) {
			r2Error("State has a global \"state\" variable with a \"willBeDestroyed\" property that is not a function.");
			destroy();
			return;
		} else if (!value->IsUndefined()) m_willBeDestroyed.Reset(isolate, LocalFunctionHandle::Cast(value));

		value = self->Get(v8str("update"));
		if (!value->IsUndefined() && !value->IsFunction()) {
			r2Error("State has a global \"state\" variable with a \"update\" property that is not a function.");
			destroy();
			return;
		} else if (!value->IsUndefined()) m_update.Reset(isolate, LocalFunctionHandle::Cast(value));

		value = self->Get(v8str("render"));
		if (!value->IsUndefined() && !value->IsFunction()) {
			r2Error("State has a global \"state\" variable with a \"render\" property that is not a function.");
			destroy();
			return;
		} else if (!value->IsUndefined()) m_render.Reset(isolate, LocalFunctionHandle::Cast(value));

		value = self->Get(v8str("handleEvent"));
		if (!value->IsUndefined() && !value->IsFunction()) {
			r2Error("State has a global \"state\" variable with a \"handleEvent\" property that is not a function.");
			destroy();
			return;
		} else if (!value->IsUndefined()) m_handleEvent.Reset(isolate, LocalFunctionHandle::Cast(value));

		m_scriptState.Reset(isolate, LocalValueHandle::Cast(self));
	}

	void state::destroy() {
		willBeDestroyed();

		if (!m_scriptState.IsEmpty()) {
			if (!m_willBecomeActive.IsEmpty()) m_willBecomeActive.Reset();
			if (!m_becameActive.IsEmpty()) m_becameActive.Reset();
			if (!m_willBecomeInactive.IsEmpty()) m_willBecomeInactive.Reset();
			if (!m_becameInactive.IsEmpty()) m_becameInactive.Reset();
			if (!m_willBeDestroyed.IsEmpty()) m_willBeDestroyed.Reset();
			m_scriptState.Reset();
		}
	}

    state_man* state::manager() const {
        return m_mgr;
    }

    void state::set_name(const string &name) {
        if(m_name.length() != 0) {
            if(m_mgr) r2Warn("Call to state::set_name failed. Attempted to rename state %s to %s",m_name.c_str(),name.c_str());
            return;
        }
        m_name = name;
    }

    string state::name() const {
        return m_name;
    }

    bool state::operator==(const state& rhs) const {
        return m_name == rhs.m_name;
    }

	void state::willBecomeActive() {
		if (m_scriptState.IsEmpty()) init();
		if (!m_willBecomeActive.IsEmpty()) m_willBecomeActive.Get(isolate)->Call(isolate->GetCurrentContext(), m_scriptState.Get(isolate), 0, NULL);
	}

	void state::becameActive() {
		m_updateTmr.reset();
		m_updateTmr.start();
		m_dt.reset();
		m_dt.start();
		if (!m_becameActive.IsEmpty()) m_becameActive.Get(isolate)->Call(isolate->GetCurrentContext(), m_scriptState.Get(isolate), 0, NULL);
	}

	void state::willBecomeInactive() {
		if (!m_willBecomeInactive.IsEmpty()) m_willBecomeInactive.Get(isolate)->Call(isolate->GetCurrentContext(), m_scriptState.Get(isolate), 0, NULL);
	}

	void state::becameInactive() {
		if (!m_becameInactive.IsEmpty()) m_becameInactive.Get(isolate)->Call(isolate->GetCurrentContext(), m_scriptState.Get(isolate), 0, NULL);
	}

	void state::willBeDestroyed() {
		if (!m_willBeDestroyed.IsEmpty()) m_willBeDestroyed.Get(isolate)->Call(isolate->GetCurrentContext(), m_scriptState.Get(isolate), 0, NULL);
	}

	void state::update() {
		if (!shouldUpdate()) return;
		if (!m_update.IsEmpty()) {
			f32 dt = m_dt;
			m_dt.reset();
			m_dt.start();
			Local<Value> arg = to_v8(isolate, dt);
			m_update.Get(isolate)->Call(isolate->GetCurrentContext(), m_scriptState.Get(isolate), 1, &arg);
			m_averageUpdateDuration += (f32)m_dt;
		}
	}

	void state::render() {
		if (!m_render.IsEmpty()) m_render.Get(isolate)->Call(isolate->GetCurrentContext(), m_scriptState.Get(isolate), 0, NULL);
	}

	void state::handle(event* evt) {
		if (!m_handleEvent.IsEmpty()) {
			auto param = Local<Value>::Cast(convert<event>::to_v8(isolate, *evt));
			m_handleEvent.Get(isolate)->Call(isolate->GetCurrentContext(), m_scriptState.Get(isolate), 1, &param);
		}
	}

	f32 state::getAverageUpdateDuration() const {
		return m_averageUpdateDuration;
	}

	bool state::shouldUpdate() {
		if (m_updateFrequency <= 0.0f) return true;
		f32 timeSinceUpdate = m_updateTmr;
		f32 waitDuration = (1.0f / m_updateFrequency);
		static f32 warnDiffFrac = 0.1f;
		static f32 warnLogInterval = 30.0f;
		static f32 warnAfterBelowForInterval = 5.0f;
		if (timeSinceUpdate >= waitDuration) {
			//printf("\"%s\": current: %0.2f Hz (%0.2fs) desired: %0.2f Hz (%0.2fs) (%0.2f%% of desired)\n", m_name.c_str(), 1.0f / timeSinceUpdate, timeSinceUpdate, m_updateFrequency, waitDuration, (waitDuration / timeSinceUpdate) * 100.0f);
			f32 delta = timeSinceUpdate - waitDuration;
			if (delta > waitDuration * warnDiffFrac) {
				if (m_timeSinceBelowFrequencyStarted.stopped()) {
					m_timeSinceBelowFrequencyStarted.start();
				}
				bool warnCondition0 = m_timeSinceBelowFrequencyStarted.elapsed() > warnAfterBelowForInterval;
				bool warnCondition1 = m_timeSinceBelowFrequencyLogged.elapsed() > warnLogInterval || m_timeSinceBelowFrequencyLogged.stopped();
				if (warnCondition0 && warnCondition1) {
					r2Warn("State \"%s\" has been updating at %0.2f%% less than the desired frequency (%0.2f Hz) for more than %0.2f seconds", m_name.c_str(), warnDiffFrac * 100.0f, m_updateFrequency, warnAfterBelowForInterval);
					if (m_timeSinceBelowFrequencyLogged.stopped()) m_timeSinceBelowFrequencyLogged.start();
					else {
						m_timeSinceBelowFrequencyLogged.reset();
						m_timeSinceBelowFrequencyLogged.start();
					}
				}
			} else if (!m_timeSinceBelowFrequencyStarted.stopped()) {
				m_timeSinceBelowFrequencyStarted.reset();
			}

			m_updateTmr.reset();
			m_updateTmr.start();
			return true;
		}

		return false;
	}




    state_man::state_man(r2engine* e) {
        m_eng = e;
		m_active = 0;
        r2Log("State manager initialized");
    }

    state_man::~state_man() {
        r2Log("State manager destroyed");
    }

    r2engine* state_man::engine() const {
        return m_eng;
    }

	bool state_man::register_state(state *s) {
		if(!s) r2Error("Call to state_man::register_state failed. Null pointer provided");

		for(auto i = m_states.begin();i != m_states.end();i++) {
			if((**i) == *s) {
				r2Error("Call to state_man::register_state failed. A state with the name \"%s\" already exists", s->m_name.c_str());
				return false;
			}
		}

		r2Log("State \"%s\" registered", s->m_name.c_str());
		m_states.push_back(s);
		return true;
	}

	bool state_man::unregister_state(state *s) {
		if(!s) r2Error("Call to state_man::unregister_state failed. Null pointer provided");

		bool found = false;
		for(auto i = m_states.begin();i != m_states.end();i++) {
			if((*i) == s) {
				if (m_active == s) {
					r2Warn("Unregistering currently active state \"%s\" may result in the engine becoming stateless", s->m_name.c_str());
					m_active->willBecomeInactive();
					m_eng->remove_child(m_active);
					m_active = 0;
					s->becameInactive();
				}
				m_states.erase(i);
				found = true;
				break;
			}
		}

		if (!found) {
			r2Error("Call to state_man::unregister_state failed. A state with the name \"%s\" doesn't exist", s->m_name.c_str());
			return false;
		}

		r2Log("State \"%s\" unregistered", s->m_name.c_str());
		return true;
	}

	state* state_man::get_state(const string& name) const {
		for(state* s : m_states) {
			if (s->m_name == name) return s;
		}

		return NULL;
	}

	void state_man::activate(const string& name) {
		state* newState = get_state(name);
		if (!newState) {
			r2Error("A state with the name \"%s\" has not been registered", name.c_str());
			return;
		}

		if (m_active) {
			state* old = m_active;
			m_active->willBecomeInactive();
			m_eng->remove_child(m_active);
			old->becameInactive();
		}
		
		newState->willBecomeActive();
		m_active = newState;
		m_eng->add_child(m_active);
		m_active->becameActive();
	}
}
