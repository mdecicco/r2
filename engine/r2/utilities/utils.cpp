#include <r2/utilities/utils.h>
#include <stdarg.h>

namespace r2 {
	mstring format_string(const char* fmt, ...) {
		va_list l;
		va_start(l, fmt);
		char buf[1024] = { 0 };
		vsnprintf(buf, 1024, fmt, l);
		mstring s = buf;
		va_end(l);
		return s;
	}
	mvector<mstring> split(const mstring& str, const mstring& sep) {
		char* cstr = const_cast<char*>(str.c_str());
		char* current;
		mvector<mstring> arr;
		current = strtok(cstr, sep.c_str());
		while (current != NULL) {
			arr.push_back(current);
			current = strtok(NULL, sep.c_str());
		}
		return arr;
	}

};
