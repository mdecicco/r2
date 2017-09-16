#ifndef STATE_MANAGER
#define STATE_MANAGER

#include <string>
#include <vector>
using namespace std;

namespace r2 {
    class r2engine;
    class state_man;
    class asset;

    class state {
        public:
            state();
            virtual ~state();

            state_man* manager() const;
            void set_name(const string& name);
            string name() const;
            bool operator==(const state& rhs) const;

            virtual vector<asset*> load_assets();


        protected:
            friend class state_man;
            state_man* m_mgr;
            string m_name;
    };

    class state_man {
        public:
            state_man(r2engine* e);
            ~state_man();

            r2engine* engine() const;

            bool register_state(state* s);
            state* get_state(const string& name) const;

        protected:
            r2engine* m_eng;
            vector<state*> m_states;
    };
}

#endif /* end of include guard: STATE_MANAGER */
