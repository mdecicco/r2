#include <r2/engine.h>
#include <stdarg.h>
#include <memory.h>
#include <r2/bindings/v8helpers.h>
#include <rlutilh.h>

#include <v8pp/convert.hpp>
#include <v8.h>
#define v8str(str) v8::String::NewFromUtf8(isolate, str)

namespace r2 {
	log_info::log_info(const mstring& _type, const mstring& _text, f32 _time) : type(_type), text(_text), time(_time) { }

	log_man::log_man() : m_start(std::chrono::steady_clock::now()) {
	}

	log_man::~log_man() {
	}

	void log_man::log(const mstring& type, const mstring& msg,...) {
		va_list l;
		va_start(l, msg);
		log(type, msg, l);
		va_end(l);
	}

	void log_man::log(const mstring& type, const mstring& msg, va_list ap) {
		memory_man::push_current(memory_man::global());
		m_lock.lock();
		switch(type[0]) {
			case 'd':
				rlutil::setColor(rlutil::GREY);
				break;
			case 'w':
				rlutil::setColor(rlutil::YELLOW);
				break;
			case 'e':
				rlutil::setColor(rlutil::LIGHTRED);
				break;
		}

		char buf[1024] = { 0 };
		vsnprintf(buf, 1024, msg.c_str(), ap);
		mstring s = mstring(buf) + '\n';
		const char* str = s.c_str();
		RLUTIL_PRINT(str);

		std::chrono::duration<f32> dt = std::chrono::high_resolution_clock::now() - m_start;
		m_lines.push_back(log_info(type, buf, dt.count()));
		rlutil::setColor(rlutil::WHITE);
		m_lock.unlock();
		memory_man::pop_current();

		r2engine* eng = r2engine::get();
		if (eng && eng->scripts()) {
			if (type == "error") {
				v8::Isolate* isolate = eng->scripts()->context()->isolate();
				if (isolate) isolate->ThrowException(v8str(""));
			}
		}
	}
}
