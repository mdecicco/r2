#include <r2/engine.h>
#include <stdarg.h>
#include <memory.h>

namespace r2 {
  log_man::log_man() {
      m_bufSz = 1024;
      m_buf = new char[m_bufSz];
      log("LOG: ","Log manager initialized | buffer length: %d bytes",m_bufSz);
  }
  log_man::~log_man() {
      log("LOG: ","Log manager destroyed");
      delete [] m_buf;
  }

  void log_man::log(const string& pre,const string& msg,...) {
      va_list l;
      va_start(l,msg);
      log(pre,msg,l);
      va_end(l);
  }
  void log_man::log(const string& pre,const string& msg,va_list ap) {
      memset(m_buf,0,m_bufSz);
      vsnprintf(m_buf,m_bufSz,msg.c_str(),ap);
      printf("%s%s\n",pre.c_str(),m_buf);
  }
}
