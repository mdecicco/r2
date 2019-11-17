#include <r2/engine.h>
#include <stdio.h>

namespace r2 {
	r2engine* r2engine::instance = 0;
	log_man r2engine::m_logger = log_man();
	void r2engine::create(int argc, char** argv) {
		if (instance) return;
		instance = new r2engine(argc, argv);
	}

    r2engine::r2engine(int argc,char** argv) : m_platform(v8::platform::NewDefaultPlatform()) {
        for(int i = 0;i < argc;i++) m_args.push_back(string(argv[i]));

		string flags = "--expose_gc";
		v8::V8::SetFlagsFromString(flags.c_str(), flags.length());
		v8::V8::InitializeExternalStartupData(argv[0]);
		v8::V8::InitializePlatform(m_platform.get());
		v8::V8::Initialize();

        m_sceneMgr = new scene_man(this);
        m_stateMgr = new state_man(this);
        m_assetMgr = new asset_man(this);
        m_fileMgr = new file_man(this);
		m_renderMgr = new render_man();
		m_scriptMgr = new script_man(this);

		string currentDir = argv[0];
		currentDir = currentDir.substr(0, currentDir.find_last_of('\\'));
		m_fileMgr->set_working_directory(currentDir);
		if(!glfwInit()) {
			glfwTerminate();
			r2Error("GLFW Could not be initialized!\n");
		}
    }
    r2engine::~r2engine() {
		m_scriptMgr->context()->isolate()->RequestGarbageCollectionForTesting(v8::Isolate::kFullGarbageCollection);

		delete m_scriptMgr; m_scriptMgr = 0;
		delete m_renderMgr; m_renderMgr = 0;
        delete m_fileMgr; m_fileMgr = 0;
        delete m_assetMgr; m_assetMgr = 0;
        delete m_stateMgr; m_stateMgr = 0;
        delete m_sceneMgr; m_sceneMgr = 0;

		v8::V8::Dispose();
		v8::V8::ShutdownPlatform();
		glfwTerminate();
    }
    void r2engine::log(const string& pre,string msg,...) {
        va_list l;
        va_start(l,msg);
        m_logger.log(pre,msg,l);
        va_end(l);
    }
    const vector<string>& r2engine::args() const {
        return m_args;
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
	log_man* r2engine::logs() const { return const_cast<log_man*>(&m_logger); }
	r2::window* r2engine::window() {
		return &m_window;
	}

	bool r2engine::open_window(i32 w, i32 h, const string& title, bool can_resize, bool fullscreen) {
		if(!m_window.create(w, h, title, can_resize, 4, 5, fullscreen)) {
			r2Error("Failed to open window \"%s\"", title.c_str());
			return false;
		}
		return true;
	}

    void r2engine::handle(event *evt) {

    }

    int r2engine::run() {
		timer frameTimer; frameTimer.start();
		f32 last_time = 0.0f;
		bool open = true;
		while(!m_window.get_close_requested()) {
			m_window.poll();
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

			scene* test = m_sceneMgr->get("scene1");
			test->render();

			ImGui_ImplGlfwGL3_NewFrame();
			if (currentState) currentState->render();

			ImGui::Render();
			ImGui::EndFrame();
			m_window.swap_buffers();
		}
        return 0;
    }

	void r2engine::shutdown() {
		// take this, good practice
		delete this;
	}
}
