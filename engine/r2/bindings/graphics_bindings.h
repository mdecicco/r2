#pragma once
#include <v8.h>

namespace r2 {
	class vertex_format;
	typedef index_type;
	class instance_format;

	bool parse_vertex(v8::Local<v8::Object>& obj, vertex_format* fmt, void* dest);
	bool parse_index(v8::Local<v8::Value>& value, index_type type, void* dest);
	bool parse_instance(v8::Local<v8::Object>& obj, instance_format* fmt, void* dest);
};