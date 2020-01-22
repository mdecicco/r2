#pragma once
#include <r2/managers/memman.h>

namespace r2 {
	mstring format_string(const char* fmt, ...);
	mvector<mstring> split(const mstring& str, const mstring& sep);
};