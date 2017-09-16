#include <r2/engine.h>

namespace r2 {
    state::state() {
    }
    state::~state() {
    }
    state_man* state::manager() const {
        return m_mgr;
    }
    void state::set_name(const string &name) {
        if(m_name.length() != 0) {
            if(m_mgr) r2Warn(m_mgr->engine(),"Call to state::set_name failed. Attempted to rename state %s to %s",m_name.c_str(),name.c_str());
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
    vector<asset*> state::load_assets() { return vector<asset*>(); }

    state_man::state_man(r2engine* e) {
        m_eng = e;
        r2Log(m_eng,"State manager initialized");
    }
    state_man::~state_man() {
        r2Log(m_eng,"State manager destroyed");
    }

    r2engine* state_man::engine() const {
        return m_eng;
    }
    bool state_man::register_state(state *s) {
        if(!s) {
            r2Error(m_eng,"Call to state_man::register_state failed. Null pointer provided");
        }
        for(auto i = m_states.begin();i != m_states.end();i++) {
            if((**i) == *s) {
                r2Error(m_eng,"Call to state_man::register_state failed. A state with the name %s already exists",s->m_name.c_str());
                return false;
            }
        }

        r2Log(m_eng,"New state registered (%s)",s->m_name.c_str());
        m_states.push_back(s);
        return true;
    }
}
