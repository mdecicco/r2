#ifndef LOG_MANAGER
#define LOG_MANAGER

#include <r2/config.h>

#include <string>
#include <stdarg.h>
using namespace std;

#ifdef r2_debug
	#include <rlutilh.h>
	#define r2Log(...) { rlutil::setColor(rlutil::GREY); r2::r2engine::get()->log("", __VA_ARGS__); rlutil::setColor(rlutil::WHITE); }

	#define r2Warn(...) { rlutil::setColor(rlutil::YELLOW); r2::r2engine::get()->log("WARNING: ", __VA_ARGS__); rlutil::setColor(rlutil::WHITE); }

	#define r2Error(...) { rlutil::setColor(rlutil::LIGHTRED); r2::r2engine::get()->log("ERROR: ", __VA_ARGS__); rlutil::setColor(rlutil::WHITE); }
#endif

namespace r2 {
    class engine;
    class log_man {
        public:
          log_man();
          ~log_man();

          void log(const string& pre,const string& msg,...);
          void log(const string& pre,const string& msg,va_list ap);
    };
}

#endif /* end of include guard: LOG_MANAGER */
