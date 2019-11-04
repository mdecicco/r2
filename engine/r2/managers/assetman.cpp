#include <r2/engine.h>

namespace r2 {
    asset::asset() {
    }
    asset::~asset() {
    }
    asset_man* asset::manager() const {
        return m_mgr;
    }
    string asset::name() const {
        return m_name;
    }
    bool asset::operator==(const asset& rhs) const {
        return m_name == rhs.m_name;
    }
    bool asset::load(const string &path) {
        r2Log("Attempting to load file %s",path.c_str());
        FILE* fp = fopen(path.c_str(),"rb");
        if(!fp) {
            r2Error("Failed to open file for reading: %s",path.c_str());
            return false;
        }
        fseek(fp,0,SEEK_END);
        size_t sz = ftell(fp);
        fseek(fp,0,SEEK_SET);

        unsigned char* data = new unsigned char[sz];
        if(fread(data,sz,1,fp) != 1) {
            r2Error("Failed to read %d bytes from file: %s",path.c_str());
            fclose(fp);
            delete [] data;
            return false;
        }
        fclose(fp);

		m_filename = path;
        bool ret = deserialize(data, sz);
		if (!ret) m_filename = "";
        delete [] data;
        return ret;
    }
    bool asset::save(const string &path) {
        r2Log("Attempting to save file %s",path.c_str());
        FILE* fp = fopen(path.c_str(),"wb");
        if(!fp) {
            r2Error("Failed to open file for writing: %s",path.c_str());
            return false;
        }

        unsigned char* data = 0;
        size_t sz = 0;
        bool ret = serialize(&data,&sz);

        if(fwrite(data,sz,1,fp) != 1) {
            r2Error("Failed to write %d bytes to file: %s",path.c_str());
            fclose(fp);
            delete [] data;
            return false;
        }
        fclose(fp);
        delete [] data;

        return ret;
    }

    asset_man::asset_man(r2engine* e) {
        m_eng = e;
        r2Log("Asset manager initialized");
    }
    asset_man::~asset_man() {
        for(auto i = m_assets.begin();i != m_assets.end();i++) {
            r2Warn("Asset %s not destroyed before engine shutdown. Destroying now to prevent memory leak",(*i)->name().c_str());
            delete *i;
        }
        m_assets.clear();
        r2Log("Asset manager destroyed");
    }
    r2engine* asset_man::engine() const {
        return m_eng;
    }
    bool asset_man::check_null(void *p) const {
        if(!p) r2Error("Call to asset_man::destroy failed. Null pointer provided");
        return !p;
    }
    bool asset_man::check_exists(const string& name,bool test,const string& msg) const {
        bool found = false;
        for(auto i = m_assets.begin();i != m_assets.end();i++) {
            if((*i)->m_name == name) {
                found = true;
                break;
            }
        }

        if(found == test) {
            r2Error(msg, name.c_str());
            return true;
        }

        return false;
    }
    void asset_man::create_success(const string& name) const {
        r2Log("New asset created (%s)", name.c_str());
    }
    void asset_man::destroy_success(const string& name) const {
        r2Log("Asset destroyed (%s)", name.c_str());
    }
}
