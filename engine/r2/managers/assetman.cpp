#include <r2/engine.h>
#include <time.h>

namespace r2 {
	engine_state_data* state_asset_factory::create() {
		return (engine_state_data*)new state_assets();
	}

	state_assets::state_assets() {
	}

	state_assets::~state_assets() {
		for(auto i = assets.begin();i != assets.end();i++) {
			delete *i;
		}

		assets.clear();
	}

    asset::asset() {
    }

    asset::~asset() {
    }

    mstring asset::name() const {
        return m_name;
    }

    bool asset::operator==(const asset& rhs) const {
        return m_name == rhs.m_name;
    }

    bool asset::load(const mstring &path) {
        r2Log("Attempting to load file %s", path.c_str());

        if (stat(path.c_str(), &m_fileStat) != 0) {
            r2Error("Failed to get stats of: %s", path.c_str());
            return false;
        }

        FILE* fp = fopen(path.c_str(),"rb");
        if(!fp) {
            r2Error("Failed to open file for reading: %s", path.c_str());
            return false;
        }
        fseek(fp, 0, SEEK_END);
        size_t sz = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        unsigned char* data = new unsigned char[sz];
        if(fread(data, sz, 1, fp) != 1) {
            r2Error("Failed to read %d bytes from file: %s", path.c_str());
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

    bool asset::save(const mstring &path) {
        r2Log("Attempting to save file %s", path.c_str());
        FILE* fp = fopen(path.c_str(), "wb");
        if(!fp) {
            r2Error("Failed to open file for writing: %s", path.c_str());
            return false;
        }

        unsigned char* data = 0;
        size_t sz = 0;
        bool ret = serialize(&data,&sz);

        if(fwrite(data,sz,1,fp) != 1) {
            r2Error("Failed to write %d bytes to file: %s", path.c_str());
            fclose(fp);
            delete [] data;
            return false;
        }
        fclose(fp);
        delete [] data;

        return ret;
    }

    void asset::reload_if_updated() {
        struct stat curStat;
        if (stat(m_filename.c_str(), &curStat) == 0) {
            if (difftime(curStat.st_mtime, m_fileStat.st_mtime) > 0) load(m_filename);
        }
    }




    asset_man::asset_man() {
        r2Log("Asset manager initialized");
    }

    asset_man::~asset_man() {
        destroy_periodic_update();
        r2Log("Asset manager destroyed");
    }

	void asset_man::initialize() {
		m_stateData = r2engine::get()->states()->register_state_data_factory<state_assets>(new state_asset_factory());
        initialize_periodic_update();
        setUpdateFrequency(1.0f / 2.0f);
        start_periodic_updates();
	}

    bool asset_man::check_exists(const mstring& name,bool test,const mstring& msg) {
		m_stateData.enable();
        bool found = false;
        for(auto i = m_stateData->assets.begin();i != m_stateData->assets.end();i++) {
            if((*i)->m_name == name) {
                found = true;
                break;
            }
        }
		m_stateData.disable();

        if(found == test) {
            r2Error(msg, name.c_str());
            return true;
        }

        return false;
    }

    void asset_man::doUpdate(f32 frameDt, f32 updateDt) {
        m_stateData.enable();
        mvector<asset*>& assets = m_stateData->assets;
        for (u32 i = 0;i < assets.size();i++) assets[i]->reload_if_updated();
        m_stateData.disable();
    }
}
