#pragma once
#include <r2/config.h>

#include <string>
#include <v8.h>


#define v8str(str) v8::String::NewFromUtf8(isolate, str)
namespace v8pp {
	class context;
};

namespace r2 {
	class r2engine;

	struct var {
		var();
		var(v8::Isolate* i, const v8::Local<v8::Value>& v);
		var(v8::Isolate* i, const std::string& jsonValue);
		template <typename T>
		var(v8::Isolate* i, T v) : isolate(i), value(v8pp::to_v8(i, v)) { }
		~var();

		operator std::string();
		operator i64();
		operator u64();
		operator i32();
		operator u32();
		operator i16();
		operator u16();
		operator i8();
		operator u8();
		operator f64();
		operator f32();
		operator bool();

		var operator[](const char* prop) const;

		v8::Isolate* isolate;
		v8::Local<v8::Value> value;
	};

	struct Vector2 {
		Vector2(f32 _x, f32 _y);
		~Vector2();

		bool operator ==(const Vector2& rhs) const;
		bool operator !=(const Vector2& rhs) const;

		f32 x;
		f32 y;
	};

	struct Vector3 {
		Vector3(f32 _x, f32 _y, f32 _z);
		~Vector3();

		bool operator ==(const Vector3& rhs) const;
		bool operator !=(const Vector3& rhs) const;

		f32 x;
		f32 y;
		f32 z;
	};

	struct Vector4 {
		Vector4(f32 _x, f32 _y, f32 _z, f32 _w);
		~Vector4();

		bool operator ==(const Vector4& rhs) const;
		bool operator !=(const Vector4& rhs) const;

		f32 x;
		f32 y;
		f32 z;
		f32 w;
	};

	struct trace {
		trace(v8::Isolate* isolate);

		std::string file;
		std::string function;
		i32 line;
	};

	typedef const v8::FunctionCallbackInfo<v8::Value>& v8Args;

	void bind_engine(v8pp::context* ctx);
	void bind_event(v8pp::context* ctx);
	void bind_math(v8pp::context* ctx);
	void bind_imgui(v8pp::context* ctx);
	void bind_graphics(v8pp::context* ctx);
}