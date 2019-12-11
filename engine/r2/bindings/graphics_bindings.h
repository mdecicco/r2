#pragma once
#include <r2/config.h>
#include <v8.h>

namespace r2 {
	class vertex_format;
	typedef index_type;
	class instance_format;

	bool parse_vertex(v8::Local<v8::Object>& obj, vertex_format* fmt, void* dest);
	bool parse_index(v8::Local<v8::Value>& value, index_type type, void* dest);
	bool parse_instance(v8::Local<v8::Object>& obj, instance_format* fmt, void* dest);
	bool parse_vertices(v8Args args, vertex_format* fmt, u8** dest, size_t* count);
	bool parse_indices(v8Args args, index_type type, u8** dest, size_t* count);
};