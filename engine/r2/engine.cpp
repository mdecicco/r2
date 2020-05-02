#include <r2/engine.h>
#include <r2/systems/scripted_sys.h>
#include <stdio.h>

namespace r2 {
	r2engine* r2engine::instance = nullptr;
	log_man* r2engine::logMgr = nullptr;
	mvector<entity_system*> r2engine::systems = mvector<entity_system*>();
	mvector<scripted_sys*> r2engine::scripted_systems = mvector<scripted_sys*>();

	state_entities::state_entities() {
		uninitializedEntities = new mvector<scene_entity*>();
	}

	state_entities::~state_entities() {
		delete uninitializedEntities;
		uninitializedEntities = nullptr;
	}

	engine_state_data* state_entities_factory::create() {
		return new state_entities();
	}



	void r2engine::register_system(entity_system* system) {
		r2engine::systems.push_back(system);
	}

	void r2engine::register_scripted_system(scripted_sys* system) {
		if (instance->m_stateMgr->m_states.size() > 0) {
			r2Error("Systems should be registered before any states are created");
			return;
		}

		r2engine::scripted_systems.push_back(system);
		system->_initialize();

		// add system as event listener
		instance->add_child(system);

		// initialize global state data
		instance->m_globalStateData.push_back(system->state().factory()->create());

		// initialize system's custom global state data
		system->scriptedState = instance->m_stateMgr->register_state_data_factory<scripted_system_state>(system->factory);
		instance->m_globalStateData.push_back(system->factory->create());
	}

	void r2engine::entity_created(scene_entity* entity) {
		for(entity_system* sys : r2engine::systems) sys->_entity_added(entity);
		for(entity_system* sys : r2engine::scripted_systems) sys->_entity_added(entity);

		/*
		if (!r2engine::instance->m_loopDidStart) {
			r2engine::instance->m_entities.enable();
			auto& entities = r2engine::instance->m_entities->entities;
			auto& updatingEntities = r2engine::instance->m_entities->updatingEntities;
			
			if (!entity->parent()) r2engine::instance->add_child(entity);
			entity->initialize();
			entities.set(entity->id(), entity);
			if (entity->doesUpdate()) updatingEntities.set(entity->id(), entity);

			r2engine::instance->m_entities.disable();
			return;
		}
		*/

		auto ref = r2engine::instance->m_entities;
		ref.enable();
		ref->uninitializedEntities->push_back(entity);
		ref.disable();
	}

	void r2engine::entity_destroyed(scene_entity* entity) {
		for(entity_system* sys : r2engine::systems) sys->_entity_removed(entity);
		for(entity_system* sys : r2engine::scripted_systems) sys->_entity_removed(entity);

		auto ref = r2engine::instance->m_entities;
		ref.enable();

		auto& entities = ref->entities;
		if (entities.has(entity->id())) entities.remove(entity->id());

		auto& updatingEntities = ref->updatingEntities;
		if (updatingEntities.has(entity->id())) updatingEntities.remove(entity->id());

		bool wasUninitialized = false;
		for (auto it = ref->uninitializedEntities->begin(); it != ref->uninitializedEntities->end(); it++) {
			if ((*it)->id() == entity->id()) {
				ref->uninitializedEntities->erase(it);
				wasUninitialized = true;
				break;
			}
		}

		if (!wasUninitialized && !entity->parent()) r2engine::instance->remove_child(entity);

		ref.disable();
	}

	void r2engine::create(int argc, char** argv) {
		if (instance) return;

		r2engine::register_system(transform_sys::get());
		r2engine::register_system(camera_sys::get());
		r2engine::register_system(mesh_sys::get());
		r2engine::register_system(physics_sys::get());
		r2engine::register_system(lighting_sys::get());

		logMgr = new log_man();
		instance = new r2engine(argc, argv);

		instance->m_assetMgr->initialize();
		instance->m_fileMgr->initialize();
		instance->m_scriptMgr->initialize();

		instance->m_entities = instance->m_stateMgr->register_state_data_factory<state_entities>(new state_entities_factory());

		for(entity_system* sys : r2engine::systems) {
			sys->_initialize();

			// add system as event listener
			instance->add_child(sys);
		}

		// initialize global state data
		for(auto factory : instance->m_stateMgr->m_engineStateDataFactories) {
			engine_state_data* data = factory->create();
			instance->m_globalStateData.push_back(data);
		}

		instance->scripts()->executeFile("./builtin.js");
	}

	r2engine* r2engine::get() { return instance; }

	v8::Isolate* r2engine::isolate() { return instance->m_scriptMgr->context()->isolate(); }

	const mvector<mstring>& r2engine::args() { return instance->m_args; }

	memory_man* r2engine::memory() { return memory_man::get(); }

	scene_man* r2engine::scenes() { return instance->m_sceneMgr; }

	state_man* r2engine::states() { return instance->m_stateMgr; }

	asset_man* r2engine::assets() { return instance->m_assetMgr; }

	file_man* r2engine::files() { return instance->m_fileMgr; }

	render_man* r2engine::renderer() { return instance->m_renderMgr; }

	script_man* r2engine::scripts() { return instance->m_scriptMgr; }

	input_man* r2engine::input() { return instance->m_inputMgr; }

	audio_man* r2engine::audio() { return instance->m_audioMgr; }

	log_man* r2engine::logs() { return logMgr; }

	interpolation_man* r2engine::interpolation() { return instance->m_interpMgr; }

	r2::window* r2engine::window() { return &instance->m_window; }

	engine_state_data* r2engine::get_engine_state_data(u16 factoryIdx) { return instance->m_globalStateData[factoryIdx]; }

	scene* r2engine::current_scene() {
		scene* debugScene = instance->m_sceneMgr->get("##debug##");
		if (debugScene) return debugScene;

		state* currentState = instance->m_stateMgr->current();
		if (currentState) return currentState->getScene();

		return nullptr;
	}

	scripted_sys* r2engine::scripted_system(const mstring& systemName) {
		for (scripted_sys* sys : instance->scripted_systems) {
			if (sys->name == systemName) return sys;
		}

		r2Error("No system with the name '%s' exists", systemName.c_str());
		return nullptr;
	}
	
	marl::Scheduler* r2engine::scheduler() {
		return &instance->m_scheduler;
	}


    r2engine::r2engine(int argc,char** argv) : m_platform(v8::platform::NewDefaultPlatform()) {
        for(int i = 0;i < argc;i++) m_args.push_back(mstring(argv[i]));

		m_fps = 0.00001f;

		mstring flags = "--expose_gc";
		v8::V8::SetFlagsFromString(flags.c_str(), flags.length());
		v8::V8::InitializeExternalStartupData(argv[0]);
		v8::V8::InitializePlatform(m_platform.get());
		v8::V8::Initialize();

		initialize_event_receiver();
        
		m_stateMgr = new state_man();
        m_assetMgr = new asset_man();
        m_fileMgr = new file_man();
		m_renderMgr = new render_man();
		m_sceneMgr = new scene_man();
		m_scriptMgr = new script_man();
		m_audioMgr = new audio_man();
		m_interpMgr = new interpolation_man();
		m_inputMgr = nullptr;
		m_loopDidStart = false;

		mstring currentDir = argv[0];
		currentDir = currentDir.substr(0, currentDir.find_last_of('\\'));
		m_fileMgr->set_working_directory(currentDir);

		if(!glfwInit()) {
			glfwTerminate();
			r2Error("GLFW Could not be initialized!\n");
		}

		m_scheduler.bind();
		m_scheduler.setWorkerThreadCount(marl::Thread::numLogicalCPUs());
    }

    r2engine::~r2engine() {
		m_scheduler.unbind();

		m_stateMgr->clearActive();				// depends on script manager, entities
		m_stateMgr->destroyStates();			// depends on script manager, entities

		destroy_all_entities();

		for (auto system : r2engine::instance->systems) {
			system->_deinitialize();
			remove_child(system);
			delete system;
		}
		for(entity_system* sys : r2engine::scripted_systems) {
			sys->_deinitialize();
			remove_child(sys);
			// should be deleted by the v8 garbage collector
		}

		if (m_inputMgr) {
			delete m_inputMgr;
			m_inputMgr = nullptr;
		}
		delete m_scriptMgr; m_scriptMgr = nullptr;	// variable dependencies (based on script usage)
        delete m_stateMgr;  m_stateMgr  = nullptr;	// depends on scene manager
        delete m_sceneMgr;  m_sceneMgr  = nullptr;	// depends on render manager, asset manager
		delete m_renderMgr; m_renderMgr = nullptr;
		delete m_fileMgr;   m_fileMgr   = nullptr;
		delete m_assetMgr;  m_assetMgr  = nullptr;
		delete m_audioMgr;  m_audioMgr  = nullptr;

		destroy_event_receiver();

		v8::V8::ShutdownPlatform();
		glfwTerminate();
    }

    void r2engine::log(const mstring& pre,mstring msg,...) {
        va_list l;
        va_start(l,msg);
        logMgr->log(pre,msg,l);
        va_end(l);
    }

	bool r2engine::open_window(i32 w, i32 h, const mstring& title, bool can_resize, bool fullscreen) {
		if(!m_window.create(w, h, title, can_resize, 4, 5, fullscreen)) {
			r2Error("Failed to open window \"%s\"", title.c_str());
			return false;
		}
		m_inputMgr = new input_man();
		return true;
	}

    void r2engine::handle(event *evt) {
		data_container* data = evt->data();
		if (data) data->set_position(0);

		if (evt->name() == EVT_NAME_ACTIVATE_STATE) {
			mstring name;
			if (!data->read_string(name, data->size())) {
				r2Error("Failed to read new state name from event data. Not changing states.");
			} else m_stateMgr->activate(name);
		} else if (evt->name() == EVT_NAME_DESTROY_ENTITY) {
			scene_entity* entity;
			if (!data->read(entity)) {
				r2Error("Failed to read pointer to entity to delete. Not deleting.");
			} else {
				entity->deferred_destroy();
				delete entity;
			}
		} else if (evt->name() == EVT_NAME_ENABLE_ENTITY_UPDATES) {
			scene_entity* entity;
			if (!data->read(entity)) {
				r2Error("Failed to read pointer to entity to delete. Not enabling updates.");
			} else {
				m_entities.enable();

				m_entities->updatingEntities.set(entity->id(), entity);

				m_entities.disable();
			}
		} else if (evt->name() == EVT_NAME_DISABLE_ENTITY_UPDATES) {
			scene_entity* entity;
			if (!data->read(entity)) {
				r2Error("Failed to read pointer to entity to delete. Not disabling updates.");
			} else {
				m_entities.enable();

				auto& updatingEntities = m_entities->updatingEntities;
				if (updatingEntities.has(entity->id())) {
					updatingEntities.remove(entity->id());
				}

				m_entities.disable();
			}
		}
    }

	void r2engine::activate_state(const mstring& name) {
		event e = evt(EVT_NAME_ACTIVATE_STATE, true, false);
		e.data()->write_string(name);
		dispatchAtFrameStart(&e);
	}
	
	void r2engine::destroy_all_entities() {
		m_entities.enable();

		auto& entities = m_entities->entities;
		entities.reverse_for_each([&entities](scene_entity** entity) {
			// This is necessary when removing elements from the array within the loop
			// since scene_entity** entity is a pointer to the array element, once it's
			// removed from the array the *entity pointer is invalidated
			scene_entity* e = *entity;
			e->deferred_destroy(); // <- entity is removed from the array by this call
			delete e;
			return true;
		});
		entities.clear();

		m_entities.disable();
	}

	void r2engine::initialize_new_entities() {
		m_entities.enable();
		for(entity_system* sys : r2engine::systems) sys->initialize_entities();
		for(entity_system* sys : r2engine::scripted_systems) sys->initialize_entities();

		auto& uninitializedEntities = *m_entities->uninitializedEntities;
		auto& entities = m_entities->entities;
		auto& updatingEntities = m_entities->updatingEntities;
		for(scene_entity* entity : uninitializedEntities) {
			if (!entity->parent()) this->add_child(entity);
			entity->initialize();
			entities.set(entity->id(), entity);
			if (entity->doesUpdate()) updatingEntities.set(entity->id(), entity);
		}
		m_entities->uninitializedEntities->clear();
		m_entities.disable();
	}

	void r2engine::update_entities(f32 dt) {
		m_entities.enable();

		m_entities->updatingEntities.for_each([dt](scene_entity** entity) {
			(*entity)->update(dt);
			return true;
		});

		m_entities.disable();
	}

    int r2engine::run() {
		m_loopDidStart = true;

		timer frameTimer; frameTimer.start();
		f32 last_time = 0.0f;

		while(!m_window.get_close_requested()) {
			// initialize any entities created last frame
			initialize_new_entities();

			// poll events from OS
			m_window.poll();

			// get input events
			if (m_inputMgr) m_inputMgr->poll();

			if (m_inputMgr && m_inputMgr->keyboard()->isKeyDown(OIS::KC_ESCAPE)) break;

			// update value animations
			m_interpMgr->update();

			// dispatch deferred events
			frame_started();

			f32 time_now = frameTimer;
			f32 dt = time_now - last_time;
			last_time = time_now;
			m_fps = 1.0f / dt;

			m_assetMgr->update(dt);

			state* currentState = m_stateMgr->current();
			if (currentState) currentState->update(dt);

			for(entity_system* sys : r2engine::systems) sys->tick(dt);
			for(entity_system* sys : r2engine::scripted_systems) sys->tick(dt);

			//timer t;
			//t.start();
			update_entities(dt);
			//f32 ut = t;
			//printf("update_entities took %f ms\n", ut * 1000.0f);

			scene* currentScene = current_scene();
			auto driver = m_renderMgr->driver();
			driver->set_viewport(vec2i(0, 0), m_window.get_size());
			if (currentScene) currentScene->render(dt);
			else driver->clear_framebuffer(vec4f(0.25f, 0.25f, 0.25f, 1.0f), true);

			ImGui_ImplGlfwGL3_NewFrame();
			if (currentState) currentState->render();

			ImGui::Render();
			ImGui::EndFrame();
			m_window.swap_buffers();
		}
        return 0;
    }

	void r2engine::shutdown() {
		delete r2engine::instance;
		delete r2engine::logMgr;

		memory_man::debug();
	}
}
