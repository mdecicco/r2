#include <r2/engine.h>

#include <v8pp/class.hpp>
#include <v8pp/utility.hpp>

using namespace v8;
using namespace v8pp;
#define v8str(str) String::NewFromUtf8(isolate, str)

namespace r2 {
    state::state(v8Args args) :
		isolate(args.GetIsolate()),
		m_name(nullptr),
		m_engineData(nullptr),
		m_memory(nullptr),
		m_scene(nullptr),
		m_desiredMemorySize(MBtoB(2)),
		m_scripted(true)
	{
		if (!r2engine::get()->renderer()->driver()) {
			r2Error("No states can be created until after a render driver has been specified");
			return;
		}

		auto global = isolate->GetCurrentContext()->Global();

		if (args.Length() != 2) {
			r2Error("No name or max memory size passed to state");
			return;
		}

		if (!args[0]->IsString()) {
			r2Error("The first parameter of state constructor should be the name of the state");
			return;
		}

		mstring name = convert<mstring>::from_v8(isolate, args[0]);
		if (name.length() == 0) {
			r2Error("State names must not be empty.");
			return;
		}

		if (r2engine::get()->states()->get_state(name)) {
			r2Error("State \"%s\" has already been registered.", name.c_str());
			return;
		}

		if (!args[1]->IsNumber()) {
			r2Error("The second parameter of state constructor should be the name of the state");
			return;
		}
		size_t maxMemSize = convert<size_t>::from_v8(isolate, args[1]);
		if (maxMemSize == 0) {
			r2Error("The maximum memory size of a state can not be zero bytes...");
			return;
		}
		if (maxMemSize > memory_man::global()->size() - memory_man::global()->used()) {
			r2Error("The specified maximum memory size exceeds the amount of available memory that can be used by anything in the application...\nEither increase the application's memory size via mem.ini or decrease this state's maximum memory size");
			return;
		}

		m_memory = new memory_allocator(maxMemSize);
		activate_allocator();
		m_name = new mstring(name);
		deactivate_allocator(true);
    }

	state::state(const mstring& name, size_t max_memory) :
		isolate(nullptr),
		m_name(nullptr),
		m_engineData(nullptr),
		m_memory(nullptr),
		m_scene(nullptr),
		m_desiredMemorySize(max_memory),
		m_scripted(false)
	{
		if (name.length() == 0) {
			r2Error("State names must not be empty.");
			return;
		}

		if (max_memory == 0) {
			r2Error("The maximum memory size of a state can not be zero bytes...");
			return;
		}

		if (max_memory > memory_man::global()->size() - memory_man::global()->used()) {
			r2Error("The specified maximum memory size exceeds the amount of available memory that can be used by anything in the application...\nEither increase the application's memory size via mem.ini or decrease this state's maximum memory size");
			return;
		}

		m_memory = new memory_allocator(max_memory);
		activate_allocator();
		m_name = new mstring(name);
		deactivate_allocator(true);
	}

    state::~state() {
		destroy();
		delete m_memory;
    }

	void state::init() {
		activate_allocator();

		m_engineData = new mvector<engine_state_data*>();
		r2engine::get()->states()->initialize_state_engine_data(this);

		initialize_periodic_update();
		initialize_event_receiver();
		m_scene = r2engine::get()->scenes()->create((*m_name) + "_scene");

		onInitialize();

		deactivate_allocator(true);
	}

	void state::init_script_data() {
		if (!m_scripted) return;

		LocalObjectHandle self = convert<state*>::to_v8(isolate, this);

		LocalValueHandle value = self->Get(v8str("willBecomeActive"));
		if (!value->IsUndefined() && !value->IsFunction()) {
			r2Error("State has a global \"state\" variable with a \"willBecomeActive\" property that is not a function.");
			deactivate_allocator(true);
			destroy();
			return;
		} else if (!value->IsUndefined()) m_willBecomeActive.Reset(isolate, LocalFunctionHandle::Cast(value));

		value = self->Get(v8str("becameActive"));
		if (!value->IsUndefined() && !value->IsFunction()) {
			r2Error("State has a global \"state\" variable with a \"becameActive\" property that is not a function.");
			deactivate_allocator(true);
			destroy();
			return;
		} else if (!value->IsUndefined()) m_becameActive.Reset(isolate, LocalFunctionHandle::Cast(value));

		value = self->Get(v8str("willBecomeInactive"));
		if (!value->IsUndefined() && !value->IsFunction()) {
			r2Error("State has a global \"state\" variable with a \"willBecomeInactive\" property that is not a function.");
			deactivate_allocator(true);
			destroy();
			return;
		} else if (!value->IsUndefined()) m_willBecomeInactive.Reset(isolate, LocalFunctionHandle::Cast(value));

		value = self->Get(v8str("becameInactive"));
		if (!value->IsUndefined() && !value->IsFunction()) {
			r2Error("State has a global \"state\" variable with a \"becameInactive\" property that is not a function.");
			deactivate_allocator(true);
			destroy();
			return;
		} else if (!value->IsUndefined()) m_becameInactive.Reset(isolate, LocalFunctionHandle::Cast(value));

		value = self->Get(v8str("willBeDestroyed"));
		if (!value->IsUndefined() && !value->IsFunction()) {
			r2Error("State has a global \"state\" variable with a \"willBeDestroyed\" property that is not a function.");
			deactivate_allocator(true);
			destroy();
			return;
		} else if (!value->IsUndefined()) m_willBeDestroyed.Reset(isolate, LocalFunctionHandle::Cast(value));

		value = self->Get(v8str("update"));
		if (!value->IsUndefined() && !value->IsFunction()) {
			r2Error("State has a global \"state\" variable with a \"update\" property that is not a function.");
			deactivate_allocator(true);
			destroy();
			return;
		} else if (!value->IsUndefined()) m_update.Reset(isolate, LocalFunctionHandle::Cast(value));

		value = self->Get(v8str("render"));
		if (!value->IsUndefined() && !value->IsFunction()) {
			r2Error("State has a global \"state\" variable with a \"render\" property that is not a function.");
			deactivate_allocator(true);
			destroy();
			return;
		} else if (!value->IsUndefined()) m_render.Reset(isolate, LocalFunctionHandle::Cast(value));

		value = self->Get(v8str("handleEvent"));
		if (!value->IsUndefined() && !value->IsFunction()) {
			r2Error("State has a global \"state\" variable with a \"handleEvent\" property that is not a function.");
			deactivate_allocator(true);
			destroy();
			return;
		} else if (!value->IsUndefined()) m_handleEvent.Reset(isolate, LocalFunctionHandle::Cast(value));

		m_scriptState.Reset(isolate, LocalValueHandle::Cast(self));
	}

	void state::destroy() {
		_willBeDestroyed();

		activate_allocator();

		r2engine::get()->destroy_all_entities();

		if (m_scripted && !m_scriptState.IsEmpty()) {
			if (!m_willBecomeActive.IsEmpty()) m_willBecomeActive.Reset();
			if (!m_becameActive.IsEmpty()) m_becameActive.Reset();
			if (!m_willBecomeInactive.IsEmpty()) m_willBecomeInactive.Reset();
			if (!m_becameInactive.IsEmpty()) m_becameInactive.Reset();
			if (!m_willBeDestroyed.IsEmpty()) m_willBeDestroyed.Reset();
			m_scriptState.Reset();
		}

		if(m_scene) {
			r2engine::get()->scenes()->destroy(m_scene);
			m_scene = nullptr;
		}

		if (m_engineData) {
			auto& engineData = *m_engineData;
			for(auto& data : engineData) {
				delete data;
			}
			delete m_engineData;
			m_engineData = nullptr;
		}

		destroy_periodic_update();
		destroy_event_receiver();

		if (m_name) { delete m_name; m_name = nullptr; }

		deactivate_allocator(true);
	}

	engine_state_data* state::get_engine_state_data(u16 factoryIdx) {
		if (m_engineData->size() == 0) return nullptr;
		return (*m_engineData)[factoryIdx];
	}

	void state::_willBecomeActive() {
		if (!m_engineData) init();
		activate_allocator();

		willBecomeActive();

		if (m_scripted) {
			if (m_scriptState.IsEmpty()) init_script_data();
			if (!m_willBecomeActive.IsEmpty()) {
				m_willBecomeActive.Get(isolate)->Call(isolate->GetCurrentContext(), m_scriptState.Get(isolate), 0, NULL);
			}
		}

		deactivate_allocator(true);
	}

	void state::_becameActive() {
		activate_allocator();

		start_periodic_updates();

		becameActive();

		if (m_scripted) {
			if (!m_becameActive.IsEmpty()) {
				m_becameActive.Get(isolate)->Call(isolate->GetCurrentContext(), m_scriptState.Get(isolate), 0, NULL);
			}
		}

		deactivate_allocator(true);
	}

	void state::_willBecomeInactive() {
		activate_allocator();

		willBecomeInactive();

		if (m_scripted) {
			if (!m_willBecomeInactive.IsEmpty()) {
				m_willBecomeInactive.Get(isolate)->Call(isolate->GetCurrentContext(), m_scriptState.Get(isolate), 0, NULL);
			}
		}

		deactivate_allocator(true);
	}

	void state::_becameInactive() {
		activate_allocator();

		stop_periodic_updates();

		becameInactive();

		if (!m_becameInactive.IsEmpty()) {
			m_becameInactive.Get(isolate)->Call(isolate->GetCurrentContext(), m_scriptState.Get(isolate), 0, NULL);
		}

		deactivate_allocator(true);
	}

	void state::_willBeDestroyed() {
		activate_allocator();

		willBeDestroyed();

		if (!m_willBeDestroyed.IsEmpty()) {
			m_willBeDestroyed.Get(isolate)->Call(isolate->GetCurrentContext(), m_scriptState.Get(isolate), 0, NULL);
		}

		deactivate_allocator(true);
	}

	void state::doUpdate(f32 frameDt, f32 updateDt) {
		activate_allocator();

		onUpdate(frameDt, updateDt);

		if (!m_update.IsEmpty()) {
			Local<Value> args[2] = {
				to_v8(isolate, frameDt),
				to_v8(isolate, updateDt)
			};
			m_update.Get(isolate)->Call(isolate->GetCurrentContext(), m_scriptState.Get(isolate), 2, args);
		}

		deactivate_allocator(true);
	}

	void state::render() {
		activate_allocator();

		onRender();

		if (!m_render.IsEmpty()) {
			m_render.Get(isolate)->Call(isolate->GetCurrentContext(), m_scriptState.Get(isolate), 0, NULL);
		}

		deactivate_allocator(true);
	}

	void state::handle(event* evt) {
		activate_allocator();

		onEvent(evt);

		if (!m_handleEvent.IsEmpty() && !evt->is_internal_only()) {
			auto param = Local<Value>::Cast(convert<event>::to_v8(isolate, *evt));
			m_handleEvent.Get(isolate)->Call(isolate->GetCurrentContext(), m_scriptState.Get(isolate), 1, &param);
		}

		deactivate_allocator(true);
	}

	void state::belowFrequencyWarning(f32 percentLessThanDesired, f32 desiredFreq, f32 timeSpentLowerThanDesired) {
		r2Warn("State \"%s\" has been updating at %0.2f%% less than the desired frequency (%0.2f Hz) for more than %0.2f seconds", m_name->c_str(), percentLessThanDesired, desiredFreq, timeSpentLowerThanDesired);
	}

	void state::releaseResources() {
		memory_man::push_current(memory_man::global());
		mstring name = mstring(m_name->c_str());
		memory_man::pop_current();

		destroy();

		m_memory->deallocate_all();

		reset_state_object_storage();

		m_name = new mstring(name.c_str());

		init();
	}

	void state::activate_allocator() {
		memory_man::push_current(m_memory);
		__set_temp_engine_state_ref(this);
	}

	void state::deactivate_allocator(bool unsetEngineStateRef) {
		memory_man::pop_current();
		if (unsetEngineStateRef) __set_temp_engine_state_ref(nullptr);
	}



    state_man::state_man() {
		m_active = nullptr;
        r2Log("State manager initialized");
    }

    state_man::~state_man() {
        r2Log("State manager destroyed");
    }

	bool state_man::register_state(state *s) {
		if(!s) r2Error("Call to state_man::register_state failed. Null pointer provided");

		for(auto i = m_states.begin();i != m_states.end();i++) {
			if((**i) == *s) {
				r2Error("Call to state_man::register_state failed. A state with the name \"%s\" already exists", s->m_name->c_str());
				return false;
			}
		}

		r2Log("State \"%s\" registered", s->m_name->c_str());
		m_states.push_back(s);
		return true;
	}

	bool state_man::unregister_state(state *s) {
		if(!s) r2Error("Call to state_man::unregister_state failed. Null pointer provided");

		bool found = false;
		for(auto i = m_states.begin();i != m_states.end();i++) {
			if((*i) == s) {
				if (m_active == s) {
					r2Warn("Unregistering currently active state \"%s\" may result in the engine becoming stateless", s->m_name->c_str());
					m_active->_willBecomeInactive();
					r2engine::get()->remove_child(m_active);
					m_active = 0;
					s->_becameInactive();
					s->releaseResources();
				}
				m_states.erase(i);
				found = true;
				break;
			}
		}

		if (!found) {
			r2Error("Call to state_man::unregister_state failed. A state with the name \"%s\" doesn't exist", s->m_name->c_str());
			return false;
		}

		r2Log("State \"%s\" unregistered", s->m_name->c_str());
		return true;
	}

	state* state_man::get_state(const mstring& name) const {
		for(state* s : m_states) {
			if ((*s->m_name) == name) return s;
		}

		return NULL;
	}
	engine_state_data_factory* state_man::factory(u16 factoryIdx) {
		return m_engineStateDataFactories[factoryIdx];
	}

	void state_man::initialize_state_engine_data(state* s) {
		for(auto factory : m_engineStateDataFactories) {
			engine_state_data* data = factory->create();
			data->m_state = s;
			s->m_engineData->push_back(data);
		}
	}

	void state_man::activate(const mstring& name) {
		state* newState = get_state(name);
		if (!newState) {
			r2Error("A state with the name \"%s\" has not been registered", name.c_str());
			return;
		}

		if (m_active) {
			state* old = m_active;
			m_active->_willBecomeInactive();
			r2engine::get()->remove_child(m_active);
			old->_becameInactive();
			old->releaseResources();
		}
		
		newState->_willBecomeActive();
		m_active = newState;
		r2engine::get()->add_child(m_active);
		m_active->_becameActive();
	}

	void state_man::clearActive() {
		if (m_active) {
			m_active->_willBecomeInactive();
			r2engine::get()->remove_child(m_active);
			m_active->_becameInactive();
			m_active->releaseResources();
			m_active = nullptr;
		}
	}

	void state_man::destroyStates() {
		for(auto state : m_states) {
			delete state;
		}
		m_states.clear();
	}
};

r2::state* v8pp::factory<r2::state, v8pp::raw_ptr_traits>::create(v8::Isolate* i, const v8::FunctionCallbackInfo<v8::Value>& args) {
	r2::state* s = new r2::state(args);
	return s;
}

void v8pp::factory<r2::state, v8pp::raw_ptr_traits>::destroy(v8::Isolate* i, r2::state* s) {
	// leave deallocating to the engine
}