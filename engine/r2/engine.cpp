#include <r2/engine.h>
#include <stdio.h>

namespace r2 {
	r2engine* r2engine::instance = nullptr;
	log_man* r2engine::logMgr = nullptr;

	void r2engine::create(int argc, char** argv) {
		if (instance) return;

		logMgr = new log_man();
		instance = new r2engine(argc, argv);

		instance->m_assetMgr->initialize();
		instance->m_fileMgr->initialize();
		instance->m_scriptMgr->initialize();

		// initialize global state data
		for(auto factory : instance->m_stateMgr->m_engineStateDataFactories) {
			engine_state_data* data = factory->create();
			instance->m_globalStateData.push_back(data);
		}

		instance->scripts()->executeFile("./builtin.min.js");
	}

    r2engine::r2engine(int argc,char** argv) : m_platform(v8::platform::NewDefaultPlatform()) {
        for(int i = 0;i < argc;i++) m_args.push_back(mstring(argv[i]));

		mstring flags = "--expose_gc";
		v8::V8::SetFlagsFromString(flags.c_str(), flags.length());
		v8::V8::InitializeExternalStartupData(argv[0]);
		v8::V8::InitializePlatform(m_platform.get());
		v8::V8::Initialize();

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
		m_stateMgr->clearActive();				// depends on script manager
		m_stateMgr->destroyStates();			// depends on script manager

		delete m_scriptMgr; m_scriptMgr = 0;	// variable dependencies (based on script usage)
        delete m_stateMgr;  m_stateMgr  = 0;	// depends on scene manager
        delete m_sceneMgr;  m_sceneMgr  = 0;	// depends on render manager, asset manager
		delete m_renderMgr; m_renderMgr = 0;
		delete m_fileMgr;   m_fileMgr   = 0;
		delete m_assetMgr;  m_assetMgr  = 0;

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

		if (evt->name() == "activate_state") {
			mstring name;
			if (!data->read_string(name, data->size())) {
				r2Error("Failed to read new state name from event data. Not changing states.");
			} else m_stateMgr->activate(name);
		}
    }

    int r2engine::run() {
		timer frameTimer; frameTimer.start();
		f32 last_time = 0.0f;

		while(!m_window.get_close_requested()) {
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
			if (currentState) {
				currentState->update();
			}

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
