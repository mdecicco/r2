#include <r2/engine.h>

#define v8str(str) v8::String::NewFromUtf8(isolate, str, v8::String::kNormalString, strlen(str))

namespace r2 {
    state::state(r2engine* eng, const string& stateScript) : m_env(eng), m_mgr(eng->states()) {
		m_script = eng->assets()->create<script, state*>("state_" + stateScript, this);
		if (!m_script->load(stateScript)) {
			r2Error("Failed to load state script \"%s\"", stateScript.c_str());
			destroy();
			return;
		}

		m_script->execute();
		
		auto isolate = m_env.context()->isolate();
		auto global = isolate->GetCurrentContext()->Global();

		LocalValueHandle stateVar = global->Get(v8str("state"));
		if (stateVar.IsEmpty()) {
			r2Error("State script \"%s\" does not declare a \"state\" variable", stateScript.c_str());
			destroy();
			return;
		}

		if (!stateVar->IsObject()) {
			r2Error("State script \"%s\" declared a global \"state\" variable, but it is not an object.", stateScript.c_str());
			destroy();
			return;
		}
		m_scriptState = LocalObjectHandle::Cast(stateVar);
		string name = *(v8::String::Utf8Value(isolate, m_scriptState->GetConstructorName()));
		if (m_mgr->get_state(name)) {
			r2Error("State script \"%s\" attempted to declare state named \"%s\", which has already been registered as a state.", stateScript.c_str(), name.c_str());
			destroy();
			return;
		}
		m_name = name;

		LocalValueHandle value = m_scriptState->Get(v8str("willBecomeActive"));
		if (!value->IsUndefined() && !value->IsFunction()) {
			r2Error("State script \"%s\" declared a global \"state\" variable with a \"willBecomeActive\" property that is not a function.", stateScript.c_str());
			destroy();
			return;
		} else if (!value->IsUndefined()) m_willBecomeActive = LocalFunctionHandle::Cast(value);

		value = m_scriptState->Get(v8str("becameActive"));
		if (!value->IsUndefined() && !value->IsFunction()) {
			r2Error("State script \"%s\" declared a global \"state\" variable with a \"becameActive\" property that is not a function.", stateScript.c_str());
			destroy();
			return;
		} else if (!value->IsUndefined()) m_becameActive = LocalFunctionHandle::Cast(value);

		value = m_scriptState->Get(v8str("willBecomeInactive"));
		if (!value->IsUndefined() && !value->IsFunction()) {
			r2Error("State script \"%s\" declared a global \"state\" variable with a \"willBecomeInactive\" property that is not a function.", stateScript.c_str());
			destroy();
			return;
		} else if (!value->IsUndefined()) m_willBecomeInactive = LocalFunctionHandle::Cast(value);

		value = m_scriptState->Get(v8str("willBeDestroyed"));
		if (!value->IsUndefined() && !value->IsFunction()) {
			r2Error("State script \"%s\" declared a global \"state\" variable with a \"willBeDestroyed\" property that is not a function.", stateScript.c_str());
			destroy();
			return;
		} else if (!value->IsUndefined()) m_willBeDestroyed = LocalFunctionHandle::Cast(value);

		value = m_scriptState->Get(v8str("update"));
		if (!value->IsUndefined() && !value->IsFunction()) {
			r2Error("State script \"%s\" declared a global \"state\" variable with a \"update\" property that is not a function.", stateScript.c_str());
			destroy();
			return;
		} else if (!value->IsUndefined()) m_update = LocalFunctionHandle::Cast(value);

		value = m_scriptState->Get(v8str("render"));
		if (!value->IsUndefined() && !value->IsFunction()) {
			r2Error("State script \"%s\" declared a global \"state\" variable with a \"render\" property that is not a function.", stateScript.c_str());
			destroy();
			return;
		} else if (!value->IsUndefined()) m_render = LocalFunctionHandle::Cast(value);

		value = m_scriptState->Get(v8str("handleEvent"));
		if (!value->IsUndefined() && !value->IsFunction()) {
			r2Error("State script \"%s\" declared a global \"state\" variable with a \"handleEvent\" property that is not a function.", stateScript.c_str());
			destroy();
			return;
		} else if (!value->IsUndefined()) m_handleEvent = LocalFunctionHandle::Cast(value);
    }

    state::~state() {
		destroy();
    }

	void state::destroy() {
		willBeDestroyed();

		if (m_script) {
			if (!m_scriptState.IsEmpty()) {
				if (!m_willBecomeActive.IsEmpty()) m_willBecomeActive.Clear();
				if (!m_becameActive.IsEmpty()) m_becameActive.Clear();
				if (!m_willBecomeInactive.IsEmpty()) m_willBecomeInactive.Clear();
				if (!m_willBeDestroyed.IsEmpty()) m_willBeDestroyed.Clear();
				m_scriptState.Clear();
			}
			m_mgr->engine()->assets()->destroy(m_script);
			m_script = 0;
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
		if (!m_willBecomeActive.IsEmpty()) {
			auto context = m_env.context()->isolate()->GetCurrentContext();
			m_willBecomeActive->Call(context, m_scriptState, 0, NULL);
		}
	}

	void state::becameActive() {
		if (!m_becameActive.IsEmpty()) {
			auto context = m_env.context()->isolate()->GetCurrentContext();
			m_becameActive->Call(context, m_scriptState, 0, NULL);
		}
	}

	void state::willBecomeInactive() {
		if (!m_willBecomeInactive.IsEmpty()) {
			auto context = m_env.context()->isolate()->GetCurrentContext();
			m_willBecomeInactive->Call(context, m_scriptState, 0, NULL);
		}
	}

	void state::willBeDestroyed() {
		if (!m_willBeDestroyed.IsEmpty()) {
			auto context = m_env.context()->isolate()->GetCurrentContext();
			m_willBeDestroyed->Call(context, m_scriptState, 0, NULL);
		}
	}

	void state::update(f32 dt) {
		if (!m_update.IsEmpty()) {
			auto context = m_env.context()->isolate()->GetCurrentContext();
			v8::Local<v8::Value> arg = v8pp::to_v8(context->GetIsolate(), dt);
			m_update->Call(context, m_scriptState, 1, &arg);
		}
	}

	void state::render() {
		if (!m_render.IsEmpty()) {
			auto context = m_env.context()->isolate()->GetCurrentContext();
			m_render->Call(context, m_scriptState, 0, NULL);
		}
	}

	void state::handle(event* evt) {
		if (!m_handleEvent.IsEmpty()) {
			if (evt->v8_local()) {
				auto context = m_env.context()->isolate()->GetCurrentContext();
				m_handleEvent->Call(context, m_scriptState, 1, (v8::Local<v8::Value>*)evt->v8_local());
			} else {
				r2Log("State \"%s\" has 'handleEvent' function, but event has no v8 data", m_name.c_str());
			}
		}
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
        if(!s) {
            r2Error("Call to state_man::register_state failed. Null pointer provided");
        }
        for(auto i = m_states.begin();i != m_states.end();i++) {
            if((**i) == *s) {
                r2Error("Call to state_man::register_state failed. A state with the name %s already exists",s->m_name.c_str());
                return false;
            }
        }

        r2Log("New state registered (%s)",s->m_name.c_str());
        m_states.push_back(s);
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
			m_active->willBecomeInactive();
			m_eng->remove_child(m_active);
		}
		
		newState->willBecomeActive();
		m_active = newState;
		m_eng->add_child(m_active);
		m_active->becameActive();
	}
}
