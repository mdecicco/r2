#pragma once

#include <r2/managers/scriptman.h>
#include <r2/managers/memman.h>
#include <r2/managers/engine_state.h>
#include <r2/utilities/event.h>
#include <r2/utilities/timer.h>
#include <r2/utilities/average.h>
#include <r2/managers/engine_state.h>
#include <r2/bindings/v8helpers.h>
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
	class scene;

    class state : public event_receiver {
        public:
            state(v8Args args);
            virtual ~state();
			
			void init();
			void destroy();

            state_man* manager() const;
			scene* getScene() const;
            mstring name() const;
            bool operator==(const state& rhs) const;

			engine_state_data* get_engine_state_data(u16 factoryIdx);

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
			void releaseResources();
			size_t getMaxMemorySize() const;
			size_t getUsedMemorySize() const;
			bool shouldUpdate();

			void activate_allocator();
			void deactivate_allocator(bool unsetEngineStateRef = false);

        protected:
            friend class state_man;
			mvector<engine_state_data*>* m_engineData;

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

			memory_allocator* m_memory;
			size_t m_desiredMemorySize;
			scene* m_scene;

			bool m_releaseResourcesOnDeactivate;
			f32 m_updateFrequency;
			average* m_averageUpdateDuration;
			timer m_updateTmr;
			timer m_dt;
			timer m_timeSinceBelowFrequencyStarted;
			timer m_timeSinceBelowFrequencyLogged;
            mstring* m_name;
    };

    class state_man {
        public:
            state_man();
            ~state_man();

            bool register_state(state* s);
			bool unregister_state(state* s);

            state* get_state(const mstring& name) const;
			state* current() { return m_active; }

			template <typename T>
			engine_state_data_ref<T> register_state_data_factory(engine_state_data_factory* factory) {
				engine_state_data_ref<T> ref(m_engineStateDataFactories.size());
				m_engineStateDataFactories.push_back(factory);
				return ref;
			}
			void initialize_state_engine_data(state* s);

			void activate(const mstring& name);
			void clearActive();
			void destroyStates();

        protected:
			friend class state;
			friend class r2engine;
            mvector<state*> m_states;
			mvector<engine_state_data_factory*> m_engineStateDataFactories;
			state* m_active;
    };
};

template <>
struct v8pp::factory<r2::state, v8pp::raw_ptr_traits> {
	static r2::state* create(v8::Isolate* i, const v8::FunctionCallbackInfo<v8::Value>& args);

	static void destroy(v8::Isolate* i, r2::state* s);
};