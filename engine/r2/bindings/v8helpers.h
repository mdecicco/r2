#pragma once
#include <r2/managers/memman.h>
#include <r2/bindings/v8pp_custom.h>

#include <v8.h>
#include <v8pp/convert.hpp>
#include <v8pp/factory.hpp>
#include <v8pp/class.hpp>

#define v8str(str) v8::String::NewFromUtf8(isolate, str)

typedef v8::Handle<v8::Value> LocalValueHandle;
typedef v8::Handle<v8::Object> LocalObjectHandle;
typedef v8::Handle<v8::Function> LocalFunctionHandle;
typedef v8::Persistent<v8::Value, v8::CopyablePersistentTraits<v8::Value>> PersistentValueHandle;
typedef v8::Persistent<v8::Object, v8::CopyablePersistentTraits<v8::Object>> PersistentObjectHandle;
typedef v8::Persistent<v8::Function, v8::CopyablePersistentTraits<v8::Function>> PersistentFunctionHandle;

namespace r2 {
	struct var {
		var();
		var(v8::Isolate* i, const v8::Local<v8::Value>& v);
		var(v8::Isolate* i, const mstring& jsonValue);
		template <typename T>
		var(v8::Isolate* i, T v) : isolate(i), value(v8pp::to_v8(i, v)) { }
		~var();

		operator mstring() const;
		operator i64() const;
		operator u64() const;
		operator i32() const;
		operator u32() const;
		operator i16() const;
		operator u16() const;
		operator i8() const;
		operator u8() const;
		operator f64() const;
		operator f32() const;
		operator bool() const;

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