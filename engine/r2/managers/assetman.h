#ifndef ASSET_MANAGER
#define ASSET_MANAGER

#include <string>
#include <vector>
using namespace std;

namespace r2 {
    class r2engine;
    class asset_man;
    class asset {
        public:
            asset_man* manager() const;
            string name() const;
            bool operator==(const asset& rhs) const;

            bool load(const string& path);
            bool save(const string& path);

            virtual bool deserialize(const unsigned char* data,size_t length) = 0;
            virtual bool serialize(unsigned char** data,size_t* length) = 0;

        protected:
            friend class asset_man;
            asset();
            virtual ~asset();

            asset_man* m_mgr;
            string m_name;
    };

    class asset_man {
        public:
            asset_man(r2engine* e);
            ~asset_man();

            r2engine* engine() const;

            template<typename t,typename ... construction_args>
            t* create(const string& name,construction_args ... args) {
                if(check_exists(name,true,"Call to asset_man::create failed. An asset with the name %s already exists")) return nullptr;

                t* a = new t(args...);
                a->m_name = name;
                a->m_mgr = this;
                m_assets.push_back(a);

                create_success(name);
                return a;
            }

            template<typename t>
            void destroy(t* a) {
                if(check_null(a)) return;
                if(check_exists(a->m_name,false,"Call to asset_man::create failed. An asset with the name %s already exists")) return;

                for(auto i = m_assets.begin();i != m_assets.end();i++) {
                    if((**i) == *a) {
                        m_assets.erase(i);
                        break;
                    }
                }

                destroy_success(a->m_name);
                delete a;
            }

        protected:
            bool check_null(void* p) const;
            bool check_exists(const string& name,bool test,const string& err) const;
            void create_success(const string& name) const;
            void destroy_success(const string& name) const;
            r2engine* m_eng;
            vector<asset*> m_assets;
    };
}
#endif /* end of include guard: ASSET_MANAGER */
