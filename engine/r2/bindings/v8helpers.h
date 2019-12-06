#pragma once
#include <r2/managers/memman.h>
#include <r2/bindings/v8pp_custom.h>

#include <v8.h>
#include <v8pp/convert.hpp>
#include <v8pp/factory.hpp>
#include <v8pp/class.hpp>

#define v8str(str) v8::String::NewFromUtf8(isolate, str)

namespace r2 {
	struct var {
		var();
		var(v8::Isolate* i, const v8::Local<v8::Value>& v);
		var(v8::Isolate* i, const mstring& jsonValue);
		template <typename T>
		var(v8::Isolate* i, T v) : isolate(i), value(v8pp::to_v8(i, v)) { }
		~var();

		operator mstring();
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

	struct trace {
		trace(v8::Isolate* isolate);

		mstring file;
		mstring function;
		i32 line;
	};

	typedef const v8::FunctionCallbackInfo<v8::Value>& v8Args;
};