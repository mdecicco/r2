#ifndef LOG_MANAGER
#define LOG_MANAGER

#include <r2/config.h>

#include <string>
#include <stdarg.h>
using namespace std;

#ifdef r2_debug
#define r2Log(engine,...) engine->log("LOG: ",__VA_ARGS__)
#define r2Warn(engine,...) engine->log("WARNING: ",__VA_ARGS__)
#define r2Error(engine,...) engine->log("ERROR: ",__VA_ARGS__)
#endif

namespace r2 {
    class engine;
    class log_man {
        public:
          log_man();
          ~log_man();

          void log(const string& pre,const string& msg,...);
          void log(const string& pre,const string& msg,va_list ap);
        protected:
          size_t m_bufSz;
          char* m_buf;
    };
}

#endif /* end of include guard: LOG_MANAGER */
