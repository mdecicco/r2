#include <r2/engine.h>
#include <stdio.h>

namespace r2 {
    r2engine::r2engine(int argc,char** argv) {
        for(int i = 0;i < argc;i++) m_args.push_back(string(argv[i]));
        m_sceneMgr = new scene_man(this);
        m_stateMgr = new state_man(this);
        m_assetMgr = new asset_man(this);
        m_fileMgr = new file_man(this);
    }
    r2engine::~r2engine() {
        delete m_fileMgr;
        delete m_assetMgr;
        delete m_stateMgr;
        delete m_sceneMgr;
    }
    void r2engine::log(const string& pre,string msg,...) {
        va_list l;
        va_start(l,msg);
        m_logger.log(pre,msg,l);
        va_end(l);
    }
    const vector<string>& r2engine::args() const {
        return m_args;
    }
    scene_man* r2engine::scenes() const {
        return m_sceneMgr;
    }
    state_man* r2engine::states() const {
        return m_stateMgr;
    }
    asset_man* r2engine::assets() const {
        return m_assetMgr;
    }
    file_man* r2engine::files() const {
        return m_fileMgr;
    }

    void r2engine::handle(event *evt) {

    }


    int r2engine::run() {
        return 0;
    }
}
