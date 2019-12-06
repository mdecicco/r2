#include <r2/bindings/v8helpers.h>

using namespace v8;
using namespace std;

namespace v8pp {
	
};

namespace r2 {
	var::var() : isolate(0) { }
	var::var(Isolate* i, const Local<Value>& v) : isolate(i), value(v) { }
	var::var(Isolate* i, const mstring& jsonValue) : isolate(i) {
		auto context = isolate->GetCurrentContext();
		auto global = context->Global();

		auto JSON = global->Get(v8str("JSON"))->ToObject(isolate);
		auto JSON_stringify = Handle<Function>::Cast(JSON->Get(v8str("parse")));
		Local<Value> param[1] = { v8str(jsonValue.c_str()) };
		JSON_stringify->Call(context, JSON, 1, param).ToLocal(&value);
	}
	var::~var() { }

	var::operator mstring() {
		auto context = isolate->GetCurrentContext();
		auto global = context->Global();
		HandleScope scope(isolate);

		Local<Value> result = value;
		if (!value->IsString()) {
			auto JSON = global->Get(v8str("JSON"))->ToObject(isolate);
			auto JSON_stringify = Handle<Function>::Cast(JSON->Get(v8str("stringify")));
			Local<Value> param[1] = { value };
			JSON_stringify->Call(context, JSON, 1, param).ToLocal(&result);
		}
		return *String::Utf8Value(isolate, result);
	}

	var::operator i64() {
		auto v = value->ToBigInt(isolate->GetCurrentContext());
		Local<Value> l;
		v.ToLocal(&l);
		auto o = l.As<BigInt>();
		return o->Int64Value();
	}

	var::operator u64() {
		auto v = value->ToBigInt(isolate->GetCurrentContext());
		Local<Value> l;
		v.ToLocal(&l);
		auto o = l.As<BigInt>();
		return o->Uint64Value();
	}

	var::operator i32() {
		auto v = *value->ToInt32(isolate);
		return v->Int32Value(isolate->GetCurrentContext()).FromJust();
	}

	var::operator u32() {
		auto v = value->ToUint32(isolate->GetCurrentContext());
		Local<Value> l;
		v.ToLocal(&l);
		return l->Uint32Value(isolate->GetCurrentContext()).FromJust();
	}

	var::operator i16() {
		auto v = *value->ToInt32(isolate);
		return v->Int32Value(isolate->GetCurrentContext()).FromJust();
	}

	var::operator u16() {
		auto v = value->ToUint32(isolate->GetCurrentContext());
		Local<Value> l;
		v.ToLocal(&l);
		return l->Uint32Value(isolate->GetCurrentContext()).FromJust();
	}

	var::operator i8() {
		auto v = *value->ToInt32(isolate);
		return v->Int32Value(isolate->GetCurrentContext()).FromJust();
	}

	var::operator u8() {
		auto v = value->ToUint32(isolate->GetCurrentContext());
		Local<Value> l;
		v.ToLocal(&l);
		return l->Uint32Value(isolate->GetCurrentContext()).FromJust();
	}

	var::operator f64() {
		auto v = *value->ToNumber(isolate);
		return v->NumberValue(isolate->GetCurrentContext()).FromJust();
	}

	var::operator f32() {
		auto v = *value->ToNumber(isolate);
		return v->NumberValue(isolate->GetCurrentContext()).FromJust();
	}

	var::operator bool() {
		auto v = value->ToBoolean(isolate->GetCurrentContext());
		Local<Value> l;
		v.ToLocal(&l);
		return *l;
	}

	var var::operator[](const char* prop) const {
		if (value->IsObject()) {
			auto v = Local<Object>::Cast(value)->Get(isolate->GetCurrentContext(), v8str(prop));
			Local<Value> local;
			v.ToLocal(&local);
			return var(isolate, local);
		}

		return *this;
	}

	trace::trace(Isolate* isolate) {
		auto trace = StackTrace::CurrentStackTrace(isolate, 1)->GetFrame(isolate, 0);
		file = *String::Utf8Value(isolate, trace->GetScriptName());
		function = *String::Utf8Value(isolate, trace->GetFunctionName());
		line = trace->GetLineNumber();
	}
};