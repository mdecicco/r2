#include <r2/engine.h>

namespace r2 {
    event::event(const mstring& file, const int line, const mstring& name, bool has_data, bool recursive) {
        m_caller.file = file;
        m_caller.line = line;
        m_name = name;
        m_recurse = recursive;
        if(has_data) {
			__set_temp_engine_state_ref(TEMP_STATE_REF__ENGINE);
			m_data = r2engine::get()->files()->create(DM_BINARY, "event_data");
			__set_temp_engine_state_ref(nullptr);
			m_internalOnly = true;
		} else m_data = nullptr;
		
    }

	event::event(const event& o) {
		m_caller.file = o.m_caller.file;
		m_caller.line = o.m_caller.line;
		m_name = o.m_name;
		m_recurse = o.m_recurse;
		m_internalOnly = o.m_internalOnly;
		m_data = o.m_data;
		const_cast<event&>(o).m_data = nullptr;
		if(!m_data && !o.m_scriptData.value.IsEmpty()) {
			mstring json = const_cast<event&>(o).m_scriptData;
			if (json.length() > 0) {
				__set_temp_engine_state_ref(TEMP_STATE_REF__ENGINE);
				m_data = r2engine::get()->files()->create(DM_TEXT, "event_json_data");
				__set_temp_engine_state_ref(nullptr);
				m_data->write_string(json);
			}
		}
	}

	event::event(v8Args args) : m_scriptData(args.Length() >= 2 ? var(args.GetIsolate(), args[1]) : var(args.GetIsolate(), "{}")) {
		m_recurse = true;
		auto isolate = args.GetIsolate();
		trace t(isolate);
		m_name = var(isolate, args[0]);
		m_caller.file = t.file;
		m_caller.line = t.line;
		m_data = nullptr;
		if (args.Length() == 3) m_recurse = var(isolate, args[2]);
	}

    event::~event() {
        if(m_data) {
			__set_temp_engine_state_ref(TEMP_STATE_REF__ENGINE);
			r2engine::get()->files()->destroy(m_data);
			__set_temp_engine_state_ref(nullptr);
		}
        m_data = nullptr;
    }

    event::caller event::emitted_at() const {
        return m_caller;
    }

    bool event::is_recursive() const {
        return m_recurse;
    }

    mstring event::name() const {
        return m_name;
    }

    data_container* event::data() const {
        return m_data;
    }

	void event::set_script_data_from_cpp(const var& v) {
		m_scriptData = v;
	}

	void event::set_script_data(v8Args args) {
		if (args.Length() != 1) args.GetIsolate()->ThrowException(v8::String::NewFromUtf8(args.GetIsolate(), "Incorrect number of arguments provided to event::set_script_data"));
		else {
			if (m_data) {
				__set_temp_engine_state_ref(TEMP_STATE_REF__ENGINE);
				r2engine::get()->files()->destroy(m_data);
				__set_temp_engine_state_ref(nullptr);

			}
			m_data = nullptr;
			m_scriptData = var(args.GetIsolate(), args[0]);
		}
	}

	v8::Local<v8::Value> event::get_script_data() {
		// This event may have been deferred, but sent from JS
		// If so, de-serialize the json string created when the
		// event was deferred and assign it to m_scriptData
		if (m_data) {
			if (m_data->name() == "event_json_data") {
				mstring json;
				m_data->set_position(0);
				if(m_data->read_string(json, m_data->size())) {
					m_scriptData = var(r2engine::get()->scripts()->context()->isolate(), json);
				}
			}
			__set_temp_engine_state_ref(TEMP_STATE_REF__ENGINE);
			r2engine::get()->files()->destroy(m_data);
			__set_temp_engine_state_ref(nullptr);
			m_data = nullptr;
		}
		return m_scriptData.value;
	}



    event_receiver::event_receiver(memory_allocator* memory) : m_memory(memory), m_isAtFrameStart(true) {
		if (!m_memory) m_memory = memory_man::global();
    }

    event_receiver::~event_receiver() {
    }

    void event_receiver::add_child(event_receiver *child) {
        m_children.push_back(child);
    }

    void event_receiver::remove_child(event_receiver* child) {
        for(auto i = m_children.begin();i != m_children.end();i++) {
            if(*i == child) {
                m_children.erase(i);
                return;
            }
        }
    }

	void event_receiver::frame_started() {
		m_isAtFrameStart = true;
		memory_man::push_current(memory_man::global());
		for (auto child : m_children) child->frame_started();
		for (auto e : m_frameStartEvents) {
			dispatch(e);
			delete e;
		}
		m_frameStartEvents.clear();
		swap(m_frameStartEvents, m_nextFrameStartEvents);
		memory_man::pop_current();
		m_isAtFrameStart = false;
	}

    void event_receiver::dispatch(event* e) {
        if(e->data()) e->data()->set_position(0);

		// ensure derived classes operate in their own memory scopes (or default to global)
		memory_man::push_current(m_memory);
        handle(e);
		memory_man::pop_current();

        if(!e->is_recursive()) return;
        for(auto i = m_children.begin();i != m_children.end();i++) {
            (*i)->dispatch(e);
        }
    }

	void event_receiver::dispatchAtFrameStart(event* e) {
		memory_man::push_current(memory_man::global());
		if (m_isAtFrameStart) {
			m_nextFrameStartEvents.push_back(new event(*e));
			return;
		}

		m_frameStartEvents.push_back(new event(*e));
		memory_man::pop_current();
	}
}
