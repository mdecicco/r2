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
		m_jsonData = o.m_jsonData;
		m_data = o.m_data;
		const_cast<event&>(o).m_data = nullptr;
	}

	event::event(v8Args args) {
		m_recurse = true;
		auto isolate = args.GetIsolate();
		trace t(isolate);
		m_name = var(isolate, args[0]);
		m_caller.file = t.file;
		m_caller.line = t.line;
		m_data = nullptr;
		m_internalOnly = false;
		if (args.Length() >= 2) {
			if (args.Length() == 3) m_recurse = var(isolate, args[2]);
			m_jsonData = var(args.GetIsolate(), args[1]);
		} else m_jsonData = "{}";
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

	void event::set_json_from_cpp(const var& v) {
		m_jsonData = v;
	}

	void event::set_json(v8Args args) {
		if (args.Length() != 1) args.GetIsolate()->ThrowException(v8::String::NewFromUtf8(args.GetIsolate(), "Incorrect number of arguments provided to event::set_json"));
		else {
			if (m_data) {
				__set_temp_engine_state_ref(TEMP_STATE_REF__ENGINE);
				r2engine::get()->files()->destroy(m_data);
				__set_temp_engine_state_ref(nullptr);

			}
			m_data = nullptr;
			m_jsonData = var(args.GetIsolate(), args[0]);
		}
	}

	v8::Local<v8::Value> event::get_json() {
		return var(r2engine::isolate(), m_jsonData).value;
	}



    event_receiver::event_receiver(memory_allocator* memory) : m_memory(memory), m_isAtFrameStart(true) {
		if (!m_memory) m_memory = memory_man::global();
    }

    event_receiver::~event_receiver() {
    }

    void event_receiver::add_child(event_receiver *child) {
        m_children.push_front(child);
    }

    void event_receiver::remove_child(event_receiver* child) {
        for(auto i = m_children.begin();i != m_children.end();i++) {
            if((*i) == child) {
                m_children.erase(i);
                return;
            }
        }
    }
	void event_receiver::subscribe(const mstring& eventName) {
		m_subscribesTo.push_front(eventName);
	}

	void event_receiver::unsubscribe(const mstring& eventName) {
		for(auto i = m_subscribesTo.begin();i != m_subscribesTo.end();i++) {
			if((*i) == eventName) {
				m_subscribesTo.erase(i);
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

		bool subscribesTo = true;
		if (m_subscribesTo.size() > 0) {
			subscribesTo = false;
			for(auto ename : m_subscribesTo) {
				if (ename == e->name()) {
					subscribesTo = true;
					break;
				}
			}
		}

		if (subscribesTo) {
			// ensure derived classes operate in their own memory scopes (or default to global)
			memory_man::push_current(m_memory);
			handle(e);
			memory_man::pop_current();
		}

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