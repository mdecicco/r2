#include <r2/engine.h>
#include <stdio.h>

#include <r2/systems/transform_sys.h>
#include <r2/systems/camera_sys.h>
#include <r2/systems/mesh_sys.h>

namespace r2 {
	r2engine* r2engine::instance = nullptr;
	log_man* r2engine::logMgr = nullptr;
	mvector<entity_system*> r2engine::systems = mvector<entity_system*>();

	state_entities::state_entities() : entities(nullptr) {
		entities = new mlist<scene_entity*>();
		uninitializedEntities = new mvector<scene_entity*>();
	}

	state_entities::~state_entities() {
		delete entities;
		entities = nullptr;
		delete uninitializedEntities;
		uninitializedEntities = nullptr;
	}

	engine_state_data* state_entities_factory::create() {
		return new state_entities();
	}



	void r2engine::register_system(entity_system* system) {
		r2engine::systems.push_back(system);
	}

	void r2engine::entity_created(scene_entity* entity) {
		for(entity_system* sys : r2engine::systems) {
			sys->_entity_added(entity);
		}

		auto ref = r2engine::instance->m_entities;
		ref.enable();
		ref->uninitializedEntities->push_back(entity);
		ref.disable();
	}

	void r2engine::entity_destroyed(scene_entity* entity) {
		for(entity_system* sys : r2engine::systems) {
			sys->_entity_removed(entity);
		}

		auto ref = r2engine::instance->m_entities;
		ref.enable();
		for(auto it = ref->entities->begin();it != ref->entities->end();it++) {
			if ((*it)->id() == entity->id()) {
				ref->entities->erase(it);
				break;
			}
		}

		bool wasUninitialized = false;
		for (auto it = ref->uninitializedEntities->begin(); it != ref->uninitializedEntities->end(); it++) {
			if ((*it)->id() == entity->id()) {
				ref->uninitializedEntities->erase(it);
				wasUninitialized = true;
				break;
			}
		}

		if (!wasUninitialized) {
			entity->unbind("wasInitialized");
			entity->unbind("handleEvent");
			entity->unbind("update");
			entity->unbind("willBeDestroyed");
			if (!entity->parent()) r2engine::instance->remove_child(entity);
		}

		ref.disable();
	}

	void r2engine::create(int argc, char** argv) {
		if (instance) return;

		r2engine::register_system(new transform_sys());
		r2engine::register_system(new camera_sys());
		r2engine::register_system(new mesh_sys());

		logMgr = new log_man();
		instance = new r2engine(argc, argv);

		instance->m_assetMgr->initialize();
		instance->m_fileMgr->initialize();
		instance->m_scriptMgr->initialize();

		instance->m_entities = instance->m_stateMgr->register_state_data_factory<state_entities>(new state_entities_factory());

		for(entity_system* sys : r2engine::systems) {
			sys->_initialize();
			instance->add_child(sys);
		}

		// initialize global state data
		for(auto factory : instance->m_stateMgr->m_engineStateDataFactories) {
			engine_state_data* data = factory->create();
			instance->m_globalStateData.push_back(data);
		}

		instance->scripts()->executeFile("./builtin.js");
	}



    r2engine::r2engine(int argc,char** argv) : m_platform(v8::platform::NewDefaultPlatform()) {
        for(int i = 0;i < argc;i++) m_args.push_back(mstring(argv[i]));

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

		mstring currentDir = argv[0];
		currentDir = currentDir.substr(0, currentDir.find_last_of('\\'));
		m_fileMgr->set_working_directory(currentDir);

		if(!glfwInit()) {
			glfwTerminate();
			r2Error("GLFW Could not be initialized!\n");
		}
    }

    r2engine::~r2engine() {
		
		m_stateMgr->clearActive();				// depends on script manager, entities
		m_stateMgr->destroyStates();			// depends on script manager, entities

		destroy_all_entities();

		for (auto system : r2engine::instance->systems) {
			system->_deinitialize();
			remove_child(system);
			delete system;
		}

		delete m_scriptMgr; m_scriptMgr = 0;	// variable dependencies (based on script usage)
        delete m_stateMgr;  m_stateMgr  = 0;	// depends on scene manager
        delete m_sceneMgr;  m_sceneMgr  = 0;	// depends on render manager, asset manager
		delete m_renderMgr; m_renderMgr = 0;
		delete m_fileMgr;   m_fileMgr   = 0;
		delete m_assetMgr;  m_assetMgr  = 0;

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

    const mvector<mstring>& r2engine::args() const {
        return m_args;
    }

	memory_man* r2engine::memory() const {
		return memory_man::get();
	}

    scene_man* r2engine::scenes() const {
        return m_sceneMgr;
    }

    state_man* r2engine::states() const {
        return m_stateMgr;
    }

    asset_man* r2engine::assets() const {
        return m_assetMgr;
    }

	file_man* r2engine::files() const {
		return m_fileMgr;
	}

	render_man* r2engine::renderer() const {
		return m_renderMgr;
	}

	script_man* r2engine::scripts() const {
		return m_scriptMgr;
	}

	log_man* r2engine::logs() const {
		return logMgr;
	}

	r2::window* r2engine::window() {
		return &m_window;
	}

	engine_state_data* r2engine::get_engine_state_data(u16 factoryIdx) {
		return m_globalStateData[factoryIdx];
	}

	scene* r2engine::current_scene() {
		scene* debugScene = m_sceneMgr->get("##debug##");
		if (debugScene) return debugScene;

		state* currentState = m_stateMgr->current();
		if (currentState) return currentState->getScene();

		return nullptr;
	}

	bool r2engine::open_window(i32 w, i32 h, const mstring& title, bool can_resize, bool fullscreen) {
		if(!m_window.create(w, h, title, can_resize, 4, 5, fullscreen)) {
			r2Error("Failed to open window \"%s\"", title.c_str());
			return false;
		}
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
				entity->destroy();
				delete entity;
			}
		}
    }
	
	void r2engine::destroy_all_entities() {
		m_entities.enable();
		for (auto entity : *m_entities->entities) {
			entity->destroy();
			delete entity;
		}
		m_entities->entities->clear();
		m_entities.disable();
	}

	void r2engine::initialize_new_entities() {
		m_entities.enable();
		for(entity_system* sys : r2engine::systems) sys->initialize_entities();

		for(scene_entity* entity : *m_entities->uninitializedEntities) {
			entity->bind("wasInitialized");
			entity->bind("handleEvent");
			entity->bind("update");
			entity->bind("willBeDestroyed");

			if (!entity->parent()) this->add_child(entity);
			entity->initialize();
			m_entities->entities->push_front(entity);
		}
		m_entities->uninitializedEntities->clear();
		m_entities.disable();
	}

	void r2engine::update_entities(f32 dt) {
		m_entities.enable();
		for(scene_entity* entity : *m_entities->entities) entity->update(dt);
		m_entities.disable();
	}

    int r2engine::run() {
		timer frameTimer; frameTimer.start();
		f32 last_time = 0.0f;

		while(!m_window.get_close_requested()) {
			// initialize any entities created last frame
			initialize_new_entities();

			// get input events
			m_window.poll();

			// dispatch deferred events
			frame_started();

			vec2i sz = m_window.get_size();
			glViewport(0, 0, sz.x, sz.y);
			glClearColor(0, 0, 0, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			f32 time_now = frameTimer;
			f32 dt = time_now - last_time;
			last_time = time_now;

			state* currentState = m_stateMgr->current();
			if (currentState) currentState->update(dt);

			for(entity_system* sys : r2engine::systems) sys->tick(dt);

			update_entities(dt);

			scene* currentScene = current_scene();
			if (currentScene) currentScene->render();

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
