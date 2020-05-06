#include <r2/engine.h>

namespace r2 {
    event::event(const mstring& file, const int line, const mstring& name, bool has_data, bool recursive) {
        m_caller.file = file;
        m_caller.line = line;
        m_name = name;
        m_recurse = recursive;
        if(has_data) {
			memory_man::push_current(memory_man::global());
			m_data = r2engine::get()->files()->create(DM_BINARY, "event_data");
			memory_man::pop_current();
			m_internalOnly = true;
		} else {
			m_data = nullptr;
			m_internalOnly = false;
		}
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
        if(m_data) r2engine::get()->files()->destroy(m_data);
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

	void event::set_json_from_str(const mstring& v) {
		m_jsonData = v;
	}

	void event::set_json(v8Args args) {
		if (args.Length() != 1) args.GetIsolate()->ThrowException(v8::String::NewFromUtf8(args.GetIsolate(), "Incorrect number of arguments provided to event::set_json"));
		else {
			if (m_data) r2engine::get()->files()->destroy(m_data);
			m_data = nullptr;
			m_jsonData = var(args.GetIsolate(), args[0]);
		}
	}

	v8::Local<v8::Value> event::get_json() {
		return var(r2engine::isolate(), m_jsonData).value;
	}



    event_receiver::event_receiver(memory_allocator* memory) : m_memory(memory), m_isAtFrameStart(true) {
		if (!m_memory) m_memory = memory_man::global();

		m_frameStartEvents = nullptr;
		m_nextFrameStartEvents = nullptr;
		m_children = nullptr;
		m_subscribesTo = nullptr;
		m_parent = nullptr;
    }

	event_receiver::event_receiver(const event_receiver& o) {
		m_memory = o.m_memory;
		m_frameStartEvents = nullptr;
		m_nextFrameStartEvents = nullptr;
		m_children = nullptr;
		m_subscribesTo = nullptr;
		if (o.m_frameStartEvents) m_frameStartEvents = new mvector<event*> ();
		if (o.m_nextFrameStartEvents) m_nextFrameStartEvents = new mvector<event*> ();
		if (o.m_children) m_children = new mlist<event_receiver*> (*o.m_children);
		if (o.m_subscribesTo) m_subscribesTo = new mlist<mstring> (*o.m_subscribesTo);
		m_parent = o.m_parent;
	}

    event_receiver::~event_receiver() {
		if (m_frameStartEvents) destroy_event_receiver();
    }

	void event_receiver::initialize_event_receiver() {
		m_lock.lock();
		m_frameStartEvents = new mvector<event*>();
		m_nextFrameStartEvents = new mvector<event*>();
		m_children = new mlist<event_receiver*>();
		m_subscribesTo = new mlist<mstring>();
		m_lock.unlock();
	}

	void event_receiver::destroy_event_receiver() {
		m_lock.lock();
		if (m_parent) {
			event_receiver* parent = m_parent;
			parent->m_lock.lock();
			for(auto i = m_parent->m_children->begin();i != m_parent->m_children->end();i++) {
				if((*i) == this) {
					m_parent->m_children->erase(i);
					m_parent = nullptr;
					break;
				}
			}
			parent->m_lock.unlock();
		}

		if (m_frameStartEvents) {
			for (auto it = m_frameStartEvents->begin();it != m_frameStartEvents->end();it++) delete *it;
			delete m_frameStartEvents;
			m_frameStartEvents = nullptr;
		}

		if (m_nextFrameStartEvents) {
			for (auto it = m_nextFrameStartEvents->begin();it != m_nextFrameStartEvents->end();it++) delete *it;
			delete m_nextFrameStartEvents;
			m_nextFrameStartEvents = nullptr;
		}

		if (m_children) delete m_children;
		m_children = nullptr;

		if (m_subscribesTo) delete m_subscribesTo;
		m_subscribesTo = nullptr;

		m_lock.unlock();
	}

    void event_receiver::add_child(event_receiver *child) {
		m_lock.lock();
        m_children->push_front(child);
		m_lock.unlock();
		child->m_lock.lock();
		child->m_parent = this;
		child->m_lock.unlock();
    }

    void event_receiver::remove_child(event_receiver* child) {
		m_lock.lock();
        for(auto i = m_children->begin();i != m_children->end();i++) {
            if((*i) == child) {
                m_children->erase(i);
				child->m_parent = nullptr;
				m_lock.unlock();
                return;
            }
        }
		m_lock.unlock();
    }

	void event_receiver::subscribe(const mstring& eventName) {
		m_lock.lock();
		m_subscribesTo->push_front(eventName);
		m_lock.unlock();
	}

	void event_receiver::unsubscribe(const mstring& eventName) {
		m_lock.lock();
		for(auto i = m_subscribesTo->begin();i != m_subscribesTo->end();i++) {
			if((*i) == eventName) {
				m_subscribesTo->erase(i);
				m_lock.unlock();
				return;
			}
		}
		m_lock.unlock();
	}

	void event_receiver::frame_started() {
		m_lock.lock();
		memory_man::push_current(memory_man::global());
		m_isAtFrameStart = true;
		for (auto it = m_children->begin();it != m_children->end();it++) (*it)->frame_started();
		for (auto it = m_frameStartEvents->begin();it != m_frameStartEvents->end();it++) {
			_dispatch(*it);
			delete *it;
		}
		//delete m_frameStartEvents;
		//m_frameStartEvents = new mvector<event*>();
		m_frameStartEvents->clear();

		swap(m_frameStartEvents, m_nextFrameStartEvents);
		m_isAtFrameStart = false;
		memory_man::pop_current();
		m_lock.unlock();
	}

    void event_receiver::dispatch(event* e) {
		m_lock.lock();
		_dispatch(e);
		m_lock.unlock();
    }

	void event_receiver::dispatchAtFrameStart(event* e) {
		m_lock.lock();
		memory_man::push_current(memory_man::global());
		if (m_isAtFrameStart) {
			_dispatch(e);
			memory_man::pop_current();
			m_lock.unlock();
			return;
		}

		m_frameStartEvents->push_back(new event(*e));
		memory_man::pop_current();
		m_lock.unlock();
	}

	void event_receiver::_dispatch(event* e) {
		bool subscribesTo = true;

		if (m_subscribesTo->size() > 0) {
			subscribesTo = false;
			auto subscriptions = *m_subscribesTo;
			for(auto ename : subscriptions) {
				if (ename == e->name()) {
					subscribesTo = true;
					break;
				}
			}
		}

		if (subscribesTo) {
			// ensure derived classes operate in their own memory scopes (or default to global)
			memory_man::push_current(m_memory);
			if(e->data()) e->data()->set_position(0);
			handle(e);
			memory_man::pop_current();
		}

		if(!e->is_recursive()) return;
		for(auto i = m_children->begin();i != m_children->end();i++) {
			(*i)->_dispatch(e);
		}
	}
}