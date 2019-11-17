#pragma once

#include <r2/managers/scriptman.h>
#include <r2/utilities/event.h>
#include <r2/utilities/timer.h>
#include <r2/utilities/average.h>

#include <string>
#include <vector>
#include <v8pp/factory.hpp>
using namespace std;

typedef v8::Handle<v8::Value> LocalValueHandle;
typedef v8::Handle<v8::Object> LocalObjectHandle;
typedef v8::Handle<v8::Function> LocalFunctionHandle;
typedef v8::Persistent<v8::Value, v8::CopyablePersistentTraits<v8::Value>> PersistentValueHandle;
typedef v8::Persistent<v8::Object, v8::CopyablePersistentTraits<v8::Object>> PersistentObjectHandle;
typedef v8::Persistent<v8::Function, v8::CopyablePersistentTraits<v8::Function>> PersistentFunctionHandle;

namespace r2 {
    class r2engine;
    class state_man;
    class asset;

    class state : public event_receiver {
        public:
            state(v8Args args);
            virtual ~state();
			
			void init();
			void destroy();

            state_man* manager() const;
            void set_name(const string& name);
            string name() const;
            bool operator==(const state& rhs) const;

			//script_env* environment() { return &m_env; }

			void willBecomeActive();
			void becameActive();
			void willBecomeInactive();
			void becameInactive();
			void willBeDestroyed();
			void update();
			void render();
			virtual void handle(event* evt);

			void setUpdateFrequency(f32 freq) { m_updateFrequency = freq; }
			f32 getAverageUpdateDuration() const;
			bool shouldUpdate();

        protected:
            friend class state_man;
            state_man* m_mgr;
			v8::Isolate* isolate;
			PersistentValueHandle m_scriptState;
			PersistentFunctionHandle m_willBecomeActive;
			PersistentFunctionHandle m_becameActive;
			PersistentFunctionHandle m_willBecomeInactive;
			PersistentFunctionHandle m_becameInactive;
			PersistentFunctionHandle m_willBeDestroyed;
			PersistentFunctionHandle m_update;
			PersistentFunctionHandle m_render;
			PersistentFunctionHandle m_handleEvent;

			f32 m_updateFrequency;
			average m_averageUpdateDuration;
			timer m_updateTmr;
			timer m_dt;
			timer m_timeSinceBelowFrequencyStarted;
			timer m_timeSinceBelowFrequencyLogged;
            string m_name;
    };

    class state_man {
        public:
            state_man(r2engine* e);
            ~state_man();

            r2engine* engine() const;

            bool register_state(state* s);
			bool unregister_state(state* s);

            state* get_state(const string& name) const;
			state* current() { return m_active; }

			void activate(const string& name);

        protected:
            r2engine* m_eng;
            vector<state*> m_states;
			state* m_active;
    };
};

template <>
struct v8pp::factory<r2::state, v8pp::raw_ptr_traits> {
	static r2::state* create(v8::Isolate* i, const v8::FunctionCallbackInfo<v8::Value>& args) {
		r2::state* s = new r2::state(args);
		//s->init();
		return s;
	}

	static void destroy(v8::Isolate* i, r2::state* s) {
		delete s;
	}
};