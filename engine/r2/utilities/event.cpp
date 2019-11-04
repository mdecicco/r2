#include <r2/engine.h>

namespace r2 {
    event::event(const string& file, const int line, r2engine* eng, const string& name, bool data_storage, bool recursive) {
        m_caller.file = file;
        m_caller.line = line;
        m_name = name;
        m_recurse = recursive;
        m_eng = eng;
		m_v8local = nullptr;
        if(recursive) {
            m_data = m_eng->files()->create(DM_BINARY, "event_data");
        } else m_data = nullptr;
    }
    event::~event() {
        if(m_data) m_eng->files()->destroy(m_data);
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
