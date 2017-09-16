#ifndef ENGINE_MAIN
#define ENGINE_MAIN

#include <string>
#include <vector>
using namespace std;

#include <r2/managers/logman.h>
#include <r2/managers/sceneman.h>
#include <r2/managers/stateman.h>
#include <r2/managers/assetman.h>
#include <r2/managers/fileman.h>

#include <r2/utilities/event.h>

namespace r2 {
    class r2engine : public event_receiver {
        public:
          r2engine(int argc,char** argv);
          ~r2engine();

          // accessors
          const vector<string>& args() const;
          scene_man* scenes() const;
          state_man* states() const;
          asset_man* assets() const;
          file_man* files() const;

          // inherited functions
          virtual void handle(event* evt);

          // debug
          void log(const string& pre,string msg,...);

          int run();

        protected:
          // managers
          log_man m_logger;
          scene_man* m_sceneMgr;
          state_man* m_stateMgr;
          asset_man* m_assetMgr;
          file_man* m_fileMgr;
          vector<string> m_args;
    };
};

#endif /* end of include guard: ENGINE_MAIN */
