#pragma once
#include <r2/managers/memman.h>
#include <r2/managers/engine_state.h>

namespace r2 {
	class asset;

	class state_asset_factory : public engine_state_data_factory {
		public:
			state_asset_factory() { }
			~state_asset_factory() { }

			virtual engine_state_data* create();
	};

	class state_assets : public engine_state_data {
		public:
			state_assets();
			~state_assets();

			mvector<asset*> assets;
	};

    class r2engine;
    class asset_man;
    class asset {
        public:
            mstring name() const;
            bool operator==(const asset& rhs) const;

            bool load(const mstring& path);
            bool save(const mstring& path);

            virtual bool deserialize(const unsigned char* data, size_t length) = 0;
            virtual bool serialize(unsigned char** data, size_t* length) = 0;

        protected:
            friend class asset_man;
			friend class state_assets;

            asset();
            virtual ~asset();

            mstring m_name;
			mstring m_filename;
    };

    class asset_man {
        public:
            asset_man();
            ~asset_man();

			void initialize();

            template<typename t,typename ... construction_args>
            t* create(const mstring& name, construction_args ... args) {
                if(check_exists(name, true, "Call to asset_man::create failed. An asset with the name %s already exists")) return nullptr;

				m_stateData.enable();
                t* a = new t(args...);
                a->m_name = name;
				m_stateData->assets.push_back(a);
				m_stateData.disable();

				r2Log("New asset created (%s)", name.c_str());
                return a;
            }

            template<typename t>
            void destroy(t* a) {
				if(!a) {
					r2Error("Call to asset_man::destroy failed. Null pointer provided");
					return;
				}

                if(check_exists(a->m_name, false, "Call to asset_man::destroy failed. An asset with the name %s does not exist")) return;

				m_stateData.enable();
                for(auto i = m_stateData->assets.begin();i != m_stateData->assets.end();i++) {
                    if((**i) == *a) {
						m_stateData->assets.erase(i);
                        break;
                    }
                }
				r2Log("Asset destroyed (%s)", a->m_name.c_str());
				delete a;
				m_stateData.disable();
            }

        protected:
            bool check_exists(const mstring& name, bool test, const mstring& err);

			engine_state_data_ref<state_assets> m_stateData;
    };
};
