#include <r2/engine.h>

namespace r2 {
    // render node
    render_node::render_node(const vtx_bo_segment& vertData,idx_bo_segment* indexData,ins_bo_segment* instanceData) {
        m_vertexData = vertData;
        if(indexData) m_indexData = idx_bo_segment(*indexData);
        if(instanceData) m_instanceData = ins_bo_segment(*instanceData);
    }
    render_node::~render_node() {
    }

    // scene
    scene::scene(scene_man* m,const string& name) {
        m_mgr = m;
        m_name = name;
        r2Log("Scene created (%s)",m_name.c_str());
    }
    scene::~scene() {
        for(auto i = m_nodes.begin();i != m_nodes.end();i++) {
            delete *i;
        }
        m_nodes.clear();
        r2Log("Scene destroyed (%s)",m_name.c_str());
    }

    scene_man* scene::manager() const {
        return m_mgr;
    }
    string scene::name() const {
        return m_name;
    }
    bool scene::operator==(const scene& rhs) const {
        return m_name == rhs.name();
    }
    bool scene::check_mesh(size_t vc) const {
        if(vc == 0) {
            r2Error("Call to scene::add_mesh failed. Mesh has no vertices");
            return false;
        }
        return true;
    }

    // manager
    scene_man::scene_man(r2engine* e) {
        m_eng = e;
        r2Log("Scene manager initialized");
    }
    scene_man::~scene_man() {
        for(auto i = m_scenes.begin();i != m_scenes.end();i++) {
            r2Warn("Scene %s not destroyed before engine shutdown. Destroying now to prevent memory leak",(*i)->name().c_str());
            delete *i;
        }
        m_scenes.clear();
        r2Log("Scene manager destroyed");
    }

    r2engine* scene_man::engine() const {
        return m_eng;
    }
    scene* scene_man::create(const string &name) {
        for(auto i = m_scenes.begin();i != m_scenes.end();i++) {
            if((*i)->name() == name) {
                r2Warn("Call to scene_man::create failed: A scene with the name \'%s\' already exists",name.c_str());
                return nullptr;
            }
        }
        scene* s = new scene(this,name);
        m_scenes.push_back(s);
        return s;
    }
    void scene_man::destroy(scene* s) {
        if(!s) {
            r2Error("Call to scene_man::destroy failed: null pointer provided");
            return;
        }

        for(auto i = m_scenes.begin();i != m_scenes.end();i++) {
            if(*(*i) == *s) {
                delete s;
                m_scenes.erase(i);
                return;
            }
        }
        r2Error("Call to scene_man::destroy failed: Provided scene not found");
    }
}
