#pragma once
#include <r2/managers/logman.h>
#include <r2/managers/sceneman.h>
#include <r2/managers/stateman.h>
#include <r2/managers/assetman.h>
#include <r2/managers/fileman.h>
#include <r2/managers/scriptman.h>
#include <r2/managers/renderman.h>
#include <r2/managers/memman.h>

#include <r2/utilities/event.h>
#include <r2/utilities/window.h>

namespace r2 {
    class r2engine : public event_receiver {
        public:
		  static void create(int argc, char** argv);
		  static r2engine* get() { return instance; }
		  static v8::Isolate* isolate() { return instance->m_scriptMgr->context()->isolate(); }

          // accessors
          const mvector<mstring>& args() const;
		  memory_man* memory() const;
          scene_man* scenes() const;
          state_man* states() const;
          asset_man* assets() const;
          file_man* files() const;
		  render_man* renderer() const;
		  script_man* scripts() const;
		  log_man* logs() const;
		  r2::window* window();

		  engine_state_data* get_engine_state_data(u16 factoryIdx);
		  scene* current_scene();

		  // functions for scripts
		  bool open_window(i32 w, i32 h, const mstring& title, bool can_resize = false, bool fullscreen = false);

          // inherited functions
          virtual void handle(event* evt);
		  
          // debug
          void log(const mstring& pre,mstring msg,...);

          int run();

		  static void shutdown();

        protected:
		  r2engine(int argc, char** argv);
		  ~r2engine();
		  static r2engine* instance;
		  static log_man* logMgr;

		  mvector<engine_state_data*> m_globalStateData;

          // managers
          scene_man* m_sceneMgr;
          state_man* m_stateMgr;
          asset_man* m_assetMgr;
          file_man* m_fileMgr;
		  render_man* m_renderMgr;
		  script_man* m_scriptMgr;

		  // stuff
		  r2::window m_window;
          mvector<mstring> m_args;

		  // v8 initialization
		  std::unique_ptr<v8::Platform> m_platform;
    };
};
