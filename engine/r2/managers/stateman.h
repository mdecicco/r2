#pragma once

#include <r2/managers/scriptman.h>
#include <r2/managers/memman.h>
#include <r2/managers/engine_state.h>
#include <r2/utilities/event.h>
#include <r2/utilities/periodic_update.h>
#include <r2/managers/engine_state.h>
#include <r2/bindings/v8helpers.h>
using namespace std;

namespace r2 {
    class r2engine;
    class state_man;
    class asset;
	class scene;

    class state : public event_receiver, public periodic_update {
        public:
            state(v8Args args);
			state(const mstring& name, size_t max_memory);
            virtual ~state();
			
			/* Accessors */
			inline scene* getScene() const { return m_scene; }
            inline mstring name() const { return *m_name; }
            inline bool operator==(const state& rhs) const { return m_name == rhs.m_name; }
			inline size_t getMaxMemorySize() const { return m_memory->size(); }
			inline size_t getUsedMemorySize() const { return m_memory->used(); }
			inline bool is_scripted() const { return m_scripted; }
			inline memory_allocator* getMemory() { return m_memory; }

			/* Functions for derived classes */
			virtual void onInitialize() { }
			virtual void willBecomeActive() { }
			virtual void becameActive() { }
			virtual void willBecomeInactive() { }
			virtual void becameInactive() { }
			virtual void willBeDestroyed() { }
			virtual void onUpdate(f32 frameDt, f32 updateDt) { }
			virtual void onRender() { }
			virtual void onEvent(event* evt) { }

			/* Used internally by engine_state_data_ref (via global functions, these can't be private) */
			void activate_allocator();
			void deactivate_allocator(bool unsetEngineStateRef = false);
			engine_state_data* get_engine_state_data(u16 factoryIdx);

        private:
            friend class state_man;
			friend class r2engine;

			void init();
			void init_script_data();
			void destroy();
			void _willBecomeActive();
			void _becameActive();
			void _willBecomeInactive();
			void _becameInactive();
			void _willBeDestroyed();
			virtual void doUpdate(f32 frameDt, f32 updateDt);
			void render();
			virtual void handle(event* evt);

			virtual void belowFrequencyWarning(f32 percentLessThanDesired, f32 desiredFreq, f32 timeSpentLowerThanDesired);

			void releaseResources();


			mvector<engine_state_data*>* m_engineData;

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
            mstring* m_name;
			bool m_scripted;
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
			engine_state_data_factory* factory(u16 factoryIdx);
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