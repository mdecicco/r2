#pragma once
using namespace r2;
using namespace v8;

namespace v8pp {
	bool _is_valid_v4(v8::Isolate* isolate, v8::Handle<v8::Value> value);

	bool _is_valid_v3(v8::Isolate* isolate, v8::Handle<v8::Value> value);

	bool _is_valid_v2(v8::Isolate* isolate, v8::Handle<v8::Value> value);

	bool _is_valid_m4(v8::Isolate* isolate, v8::Handle<v8::Value> value);

	bool _is_valid_m3(v8::Isolate* isolate, v8::Handle<v8::Value> value);

	bool _is_valid_m2(v8::Isolate* isolate, v8::Handle<v8::Value> value);

	#define vc(t, postfix0, postfix1) \
		t _from_v8_##postfix0##postfix1(v8::Isolate* isolate, v8::Local<v8::Value> value); \
		v8::Handle<v8::Object> _to_v8_##postfix0##postfix1(v8::Isolate* isolate, t const& value); \
		\
		template<> \
		struct convert<t> { \
			using from_type = t; \
			using to_type = v8::Handle<v8::Object>; \
			\
			static bool is_valid(v8::Isolate* isolate, v8::Handle<v8::Value> value) { return _is_valid_##postfix0(isolate, value); } \
			static t from_v8(v8::Isolate* isolate, v8::Local<v8::Value> value) { return _from_v8_##postfix0##postfix1(isolate, value); } \
			static v8::Handle<v8::Object> to_v8(v8::Isolate* isolate, t const& value) { return _to_v8_##postfix0##postfix1(isolate, value); } \
		};

		vc(vec4i , v4, i);
		vc(vec4ui, v4, ui);
		vc(vec4f , v4, f);
		vc(vec3i , v3, i);
		vc(vec3ui, v3, ui);
		vc(vec3f , v3, f);
		vc(vec2i , v2, i);
		vc(vec2ui, v2, ui);
		vc(vec2f , v2, f);
		vc(mat4i , m4, i);
		vc(mat4ui, m4, ui);
		vc(mat4f , m4, f);
		vc(mat3i , m3, i);
		vc(mat3ui, m3, ui);
		vc(mat3f , m3, f);
		vc(mat2i , m2, i);
		vc(mat2ui, m2, ui);
		vc(mat2f , m2, f);
	#undef vc
};