#pragma once
#include <r2/managers/memman.h>

#include <stdarg.h>
#include <chrono>
using namespace std;

namespace r2 {
    class engine;
	class event;
	struct log_info {
		log_info(const mstring& type, const mstring& text, f32 time);
		f32 time;
		mstring type;
		mstring text;
	};
    class log_man {
        public:
			log_man();
			~log_man();

			void log(const mstring& type,const mstring& msg,...);
			void log(const mstring& type,const mstring& msg,va_list ap);

			mvector<log_info> lines() const { return m_lines; }

		protected:
			mvector<log_info> m_lines;
			std::chrono::high_resolution_clock::time_point m_start;
    };
};