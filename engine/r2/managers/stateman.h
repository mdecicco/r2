#ifndef STATE_MANAGER
#define STATE_MANAGER

#include <r2/managers/scriptman.h>
#include <r2/utilities/event.h>

#include <string>
#include <vector>
using namespace std;

typedef v8::Handle<v8::Value> LocalValueHandle;
typedef v8::Handle<v8::Object> LocalObjectHandle;
typedef v8::Handle<v8::Function> LocalFunctionHandle;

namespace r2 {
    class r2engine;
    class state_man;
    class asset;

    class state : public event_receiver {
        public:
            state(r2engine* eng, const string& stateScript);
            virtual ~state();

			void destroy();

            state_man* manager() const;
            void set_name(const string& name);
            string name() const;
            bool operator==(const state& rhs) const;

			script_env* environment() { return &m_env; }

			void willBecomeActive();
			void becameActive();
			void willBecomeInactive();
			void willBeDestroyed();
			void update(f32 dt);
			void render();
			virtual void handle(event* evt);

        protected:
            friend class state_man;
			script* m_script;
            state_man* m_mgr;
			script_env m_env;
			LocalObjectHandle m_scriptState;
			LocalFunctionHandle m_willBecomeActive;
			LocalFunctionHandle m_becameActive;
			LocalFunctionHandle m_willBecomeInactive;
			LocalFunctionHandle m_willBeDestroyed;
			LocalFunctionHandle m_update;
			LocalFunctionHandle m_render;
			LocalFunctionHandle m_handleEvent;
            string m_name;
    };

    class state_man {
        public:
            state_man(r2engine* e);
            ~state_man();

            r2engine* engine() const;

            bool register_state(state* s);

            state* get_state(const string& name) const;
			state* current() { return m_active; }

			void activate(const string& name);

        protected:
            r2engine* m_eng;
            vector<state*> m_states;
			state* m_active;
    };
}

#endif /* end of include guard: STATE_MANAGER */
