#pragma once
#include <r2/managers/logman.h>
#include <r2/managers/sceneman.h>
#include <r2/managers/stateman.h>
#include <r2/managers/assetman.h>
#include <r2/managers/fileman.h>
#include <r2/managers/scriptman.h>
#include <r2/managers/renderman.h>
#include <r2/managers/inputman.h>
#include <r2/managers/audioman.h>
#include <r2/managers/memman.h>
#include <r2/managers/interpolationman.h>

#include <r2/systems/entity.h>
#include <r2/systems/transform_sys.h>
#include <r2/systems/camera_sys.h>
#include <r2/systems/mesh_sys.h>
#include <r2/systems/physics_sys.h>
#include <r2/systems/lighting_sys.h>
#include <r2/systems/animation_sys.h>

#include <r2/utilities/event.h>
#include <r2/utilities/window.h>

#include <marl/scheduler.h>

namespace r2 {
	class scripted_sys;
	class state_entities : public engine_state_data {
		public:
			state_entities();
			~state_entities();

			associative_pod_array<entityId, scene_entity*> entities;
			associative_pod_array<entityId, scene_entity*> updatingEntities;
			mvector<scene_entity*>* uninitializedEntities;
	};

	class state_entities_factory : public engine_state_data_factory {
		public:
			state_entities_factory() { }
			~state_entities_factory() { }

			virtual engine_state_data* create();
	};

    class r2engine : public event_receiver {
        public:
			static void register_system(entity_system* system);
			static void register_scripted_system(scripted_sys* system);
			static void entity_created(scene_entity* entity);
			static void entity_destroyed(scene_entity* entity);
			static void create(int argc, char** argv);
			static r2engine* get();
			static v8::Isolate* isolate();

			// accessors
			static const mvector<mstring>& args();
			static memory_man* memory();
			static scene_man* scenes();
			static state_man* states();
			static asset_man* assets();
			static file_man* files();
			static render_man* renderer();
			static script_man* scripts();
			static input_man* input();
			static audio_man* audio();
			static log_man* logs();
			static interpolation_man* interpolation();
			static scripted_sys* scripted_system(const mstring& systemName);
			static marl::Scheduler* scheduler();

			static r2::window* window();
			static engine_state_data* get_engine_state_data(u16 factoryIdx);
			static scene* current_scene();
			static inline f32 fps() { return instance->m_fps; }
			static bool started() { return instance->m_loopDidStart; }
			static scene_entity* entity(entityId);

			// description not found
			bool open_window(i32 w, i32 h, const mstring& title, bool can_resize = false, bool fullscreen = false);
			void log(const mstring& pre, mstring msg,...);
			void activate_state(const mstring& name);
			void destroy_all_entities();

			// inherited functions
			virtual void handle(event* evt);

			// loop functions
			void initialize_new_entities();
			void update_entities(f32 dt);

			// loop
			int run();

			static void shutdown();

        protected:
			r2engine(int argc, char** argv);
			~r2engine();
			static r2engine* instance;
			static log_man* logMgr;
			static mvector<entity_system*> systems;
			static mvector<scripted_sys*> scripted_systems;

			mvector<engine_state_data*> m_globalStateData;
			engine_state_data_ref<state_entities> m_entities;

			// managers
			scene_man* m_sceneMgr;
			state_man* m_stateMgr;
			asset_man* m_assetMgr;
			file_man* m_fileMgr;
			render_man* m_renderMgr;
			script_man* m_scriptMgr;
			input_man* m_inputMgr;
			audio_man* m_audioMgr;
			interpolation_man* m_interpMgr;

			// stuff
			r2::window m_window;
			mvector<mstring> m_args;
			f32 m_fps;
			bool m_loopDidStart;

			// task management
			marl::Scheduler m_scheduler;

			// v8 initialization
			std::unique_ptr<v8::Platform> m_platform;
    };
};
