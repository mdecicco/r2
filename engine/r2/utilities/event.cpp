#include <r2/engine.h>

namespace r2 {
    event::event(const string& file, const int line, const string& name, bool has_data, bool recursive) {
        m_caller.file = file;
        m_caller.line = line;
        m_name = name;
        m_recurse = recursive;
		m_v8local = nullptr;
        if(has_data) m_data = r2engine::get()->files()->create(DM_BINARY, "event_data");
        else m_data = nullptr;
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
        if(m_data) r2engine::get()->files()->destroy(m_data);
        m_data = nullptr;
    }

    event::caller event::emitted_at() const {
        return m_caller;
    }
    bool event::is_recursive() const {
        return m_recurse;
    }
    string event::name() const {
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
		else m_scriptData = var(args.GetIsolate(), args[0]);
	}
	v8::Local<v8::Value> event::get_script_data() const {
		return m_scriptData.value;
	}

    event_receiver::event_receiver() {
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
    void event_receiver::dispatch(event* e) {
        if(e->data()) e->data()->set_position(0);
        handle(e);
        if(!e->is_recursive()) return;
        for(auto i = m_children.begin();i != m_children.end();i++) {
            (*i)->dispatch(e);
        }
    }
}
