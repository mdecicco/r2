#include <r2/bindings/bindings.h>
#include <r2/engine.h>

using namespace v8;
using namespace v8pp;

#define v8str(str) v8::String::NewFromUtf8(isolate, str, v8::String::kNormalString, strlen(str))

namespace r2 {
	var::var(Isolate* i, const Local<Value>& v) : isolate(i), value(v) { }
	var::var(Isolate* i, const string& jsonValue) : isolate(i) {
		auto context = isolate->GetCurrentContext();
		auto global = context->Global();

		auto JSON = global->Get(v8str("JSON"))->ToObject(isolate);
		auto JSON_stringify = Handle<Function>::Cast(JSON->Get(v8str("parse")));
		Local<Value> param[1] = { v8str(jsonValue.c_str()) };
		JSON_stringify->Call(context, JSON, 1, param).ToLocal(&value);
	}
	var::~var() { }

	var::operator string() {
		if (value->IsObject()) {
			auto context = isolate->GetCurrentContext();
			auto global = context->Global();
			HandleScope scope(isolate);

			auto JSON = global->Get(v8str("JSON"))->ToObject(isolate);
			auto JSON_stringify = Handle<Function>::Cast(JSON->Get(v8str("stringify")));
			Local<Value> param[1] = { value };
			Local<Value> result;
			JSON_stringify->Call(context, JSON, 1, param).ToLocal(&result);
			return *String::Utf8Value(isolate, result);
		}
		return *String::Utf8Value(isolate, value);
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
	}

	trace::trace(Isolate* isolate) {
		auto trace = StackTrace::CurrentStackTrace(isolate, 1)->GetFrame(isolate, 0);
		file = *String::Utf8Value(isolate, trace->GetScriptName());
		function = *String::Utf8Value(isolate, trace->GetFunctionName());
		line = trace->GetLineNumber();
	}

	void log(v8Args args) {
		r2engine* engine = r2engine::get();
		auto isolate = args.GetIsolate();
		auto context = args.GetIsolate()->GetCurrentContext();
		auto global = context->Global();

		string text;
		for (u8 i = 0; i < args.Length(); i++) {
			if (i > 0) text += " ";
			text += var(isolate, args[i]);
		}

		r2Log(text.c_str());
	}

	void warn(v8Args args) {
		r2engine* engine = r2engine::get();
		auto isolate = args.GetIsolate();
		auto context = args.GetIsolate()->GetCurrentContext();
		auto global = context->Global();

		string text;
		for (u8 i = 0; i < args.Length(); i++) {
			if (i > 0) text += " ";
			if (args[i]->IsObject()) {
				HandleScope scope(isolate);

				auto JSON = global->Get(v8str("JSON"))->ToObject(isolate);
				auto JSON_stringify = Handle<Function>::Cast(JSON->Get(v8str("stringify")));
				Local<Value> param[1] = { args[i] };
				Local<Value> result;
				JSON_stringify->Call(context, JSON, 1, param).ToLocal(&result);
				text += var(isolate, result);
			} else text += var(isolate, args[i]);
		}

		r2Warn(text.c_str());
	}

	void error(v8Args args) {
		r2engine* engine = r2engine::get();
		auto isolate = args.GetIsolate();
		auto context = args.GetIsolate()->GetCurrentContext();
		auto global = context->Global();

		string text;
		for (u8 i = 0; i < args.Length(); i++) {
			if (i > 0) text += " ";
			if (args[i]->IsObject()) {
				HandleScope scope(isolate);

				auto JSON = global->Get(v8str("JSON"))->ToObject(isolate);
				auto JSON_stringify = Handle<Function>::Cast(JSON->Get(v8str("stringify")));
				Local<Value> param[1] = { args[i] };
				Local<Value> result;
				JSON_stringify->Call(context, JSON, 1, param).ToLocal(&result);
				text += var(isolate, result);
			} else text += var(isolate, args[i]);
		}

		r2Error(text.c_str());
	}

	void dispatch(v8Args args) {
		r2engine* engine = r2engine::get();
		auto isolate = args.GetIsolate();
		auto context = isolate->GetCurrentContext();
		auto global = context->Global();

		auto s = unwrapArg<event>(args[0]);
		s->set_v8_local(&args[0]);
		engine->dispatch(s);
		s->set_v8_local(nullptr);
	}

	void open_window(v8Args args) {
		r2engine* engine = r2engine::get();
		auto isolate = args.GetIsolate();
		auto context = isolate->GetCurrentContext();
		auto global = context->Global();

		if (args.Length() < 1 || !args[0]->IsObject()) {
			isolate->ThrowException(v8str("engine.open_window must receive an object with at least 3 properties: width, height, and title. (Optionally: can_resize, fullscreen)"));
			args.GetReturnValue().Set(false);
			return;
		}

		auto options = Local<Object>::Cast(args[0]);
		auto width = options->Get(v8str("width"));
		auto height = options->Get(v8str("height"));
		auto title = options->Get(v8str("title"));
		auto can_resize = options->Get(v8str("can_resize"));
		auto fullscreen = options->Get(v8str("fullscreen"));

		i32 v_width = 0;
		i32 v_height = 0;
		string v_title = "";
		bool v_can_resize = false;
		bool v_fullscreen = false;

		u32 max_width = 0, max_height = 0;
		engine->window()->get_max_resolution(max_width, max_height);

		if (!width->IsInt32()) {
			isolate->ThrowException(v8str("engine.open_window: options.width must be an integer"));
			args.GetReturnValue().Set(false);
			return;
		}
		v_width = var(isolate, width);
		if(v_width == 0) v_width = max_width;

		if (v_width < 1 || v_width > max_width) {
			isolate->ThrowException(v8str("engine.open_window: options.width must be between 1 and the width of the monitor in pixels, or 0 for the width of the monitor in pixels"));
			args.GetReturnValue().Set(false);
			return;
		}


		if (!height->IsInt32()) {
			isolate->ThrowException(v8str("engine.open_window: options.height must be an integer"));
			args.GetReturnValue().Set(false);
			return;
		}
		v_height = var(isolate, height);
		if(v_height == 0) v_width = max_height;

		if (v_height < 1 || v_height > max_height) {
			isolate->ThrowException(v8str("engine.open_window: options.height must be between 1 and the height of the monitor in pixels, or 0 for the height of the monitor in pixels"));
			args.GetReturnValue().Set(false);
			return;
		}

		if (!title->IsString()) {
			isolate->ThrowException(v8str("engine.open_window: options.title must be a string"));
			args.GetReturnValue().Set(false);
			return;
		}
		v_title = var(isolate, title);

		if (!can_resize->IsNullOrUndefined()) {
			if(!can_resize->IsBoolean()) {
				isolate->ThrowException(v8str("engine.open_window: options.can_resize must be a boolean"));
				args.GetReturnValue().Set(false);
				return;
			}
			v_can_resize = var(isolate, can_resize);
		}

		if (!fullscreen->IsNullOrUndefined()) {
			if (!fullscreen->IsBoolean()) {
				isolate->ThrowException(v8str("engine.open_window: options.fullscreen must be a boolean"));
				args.GetReturnValue().Set(false);
				return;
			}
			v_fullscreen = var(isolate, fullscreen);
		}

		args.GetReturnValue().Set(engine->open_window(v_width, v_height, v_title, v_can_resize, v_fullscreen));
	}

	void bind_engine(r2engine* eng, context* ctx) {
		bind_event(eng, ctx);
		bind_math(ctx);
		bind_imgui(ctx);

		auto isolate = ctx->isolate();
		auto context = isolate->GetCurrentContext();
		auto global = context->Global();

		v8::Handle<v8::ObjectTemplate> t = ObjectTemplate::New(isolate);
		t->SetInternalFieldCount(1);
		t->Set(v8str("log"), FunctionTemplate::New(isolate, log));
		t->Set(v8str("warn"), FunctionTemplate::New(isolate, warn));
		t->Set(v8str("error"), FunctionTemplate::New(isolate, error));
		t->Set(v8str("dispatch"), FunctionTemplate::New(isolate, dispatch));
		t->Set(v8str("open_window"), FunctionTemplate::New(isolate, open_window));
		v8::MaybeLocal<v8::Object> maybeLocalSelf = t->NewInstance(context);
		LocalObjectHandle self;
		maybeLocalSelf.ToLocal(&self);

		global->Set(v8str("engine"), self);
	}
}