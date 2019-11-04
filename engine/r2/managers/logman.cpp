#include <r2/engine.h>
#include <stdarg.h>
#include <memory.h>

namespace r2 {
  log_man::log_man() {
      r2Log("Log manager initialized");
  }
  log_man::~log_man() {
      r2Log("Log manager destroyed");
  }

  void log_man::log(const string& pre, const string& msg,...) {
      va_list l;
      va_start(l, msg);
      log(pre, msg, l);
      va_end(l);
  }
  void log_man::log(const string& pre, const string& msg, va_list ap) {
      char buf[1024] = { 0 };
      vsnprintf(buf, 1024, msg.c_str(), ap);
	  std::string s = pre + buf + "\n";
	  const char* str = s.c_str();
      RLUTIL_PRINT(str);
  }
}
