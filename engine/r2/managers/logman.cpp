#include <r2/engine.h>
#include <stdarg.h>
#include <memory.h>

namespace r2 {
	log_info::log_info(const string& _type, const string& _text, f32 _time) : type(_type), text(_text), time(_time) { }

	log_man::log_man() : m_start(std::chrono::steady_clock::now()) {
	}
	log_man::~log_man() {
	}

	void log_man::log(const string& type, const string& msg,...) {
		va_list l;
		va_start(l, msg);
		log(type, msg, l);
		va_end(l);
	}
	void log_man::log(const string& type, const string& msg, va_list ap) {
		char buf[1024] = { 0 };
		vsnprintf(buf, 1024, msg.c_str(), ap);
		std::string s = string(buf) + '\n';
		const char* str = s.c_str();
		RLUTIL_PRINT(str);

		std::chrono::duration<f32> dt = std::chrono::high_resolution_clock::now() - m_start;
		m_lines.push_back(log_info(type, buf, dt.count()));

		r2engine* eng = r2engine::get();
		if (eng && eng->scripts()) {
			event e(__FILE__, __LINE__, "log", false, true);
			e.set_script_data_from_cpp(var(eng->scripts()->context()->isolate(), m_lines[m_lines.size() - 1]));
			r2engine::get()->dispatch(&e);
		}
	}
}
