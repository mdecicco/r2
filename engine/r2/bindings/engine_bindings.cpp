#include <r2/bindings/bindings.h>
#include <r2/engine.h>
#include <v8pp/class.hpp>
#include <v8pp/json.hpp>
#include <v8pp/convert.hpp>

using namespace v8;
using namespace v8pp;

namespace r2 {
	var::var() : isolate(0) { }
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

		return *this;
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

	void dispatch(const event& evt) {
		r2engine* engine = r2engine::get();
		engine->dispatch(const_cast<event*>(&evt));
	}

	void logs(v8Args args) {
		auto lines = r2engine::get()->logs()->lines();
		args.GetReturnValue().Set(to_v8(args.GetIsolate(), lines));
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

	void register_state(state* s) {
		r2engine::get()->states()->register_state(s);
	}

	void activate_state(const string& stateName) {
		r2engine::get()->states()->activate(stateName);
	}

	void bind_engine(context* ctx) {
		bind_event(ctx);
		bind_math(ctx);
		bind_imgui(ctx);
		bind_graphics(ctx);

		auto isolate = ctx->isolate();
		auto context = isolate->GetCurrentContext();
		auto global = context->Global();

		Local<Array> states = Local<Array>::New(isolate, Array::New(isolate));
		global->Set(v8str("_states"), states);

		module m(isolate);

		{
			class_<state, raw_ptr_traits> s(isolate);
			s.ctor<v8Args>();
			s.set("set_update_frequency", &state::setUpdateFrequency);
			s.set("get_average_update_duration", &state::getAverageUpdateDuration);
			m.set("State", s);
		}

		m.set("log", &log);
		m.set("warn", &warn);
		m.set("error", &error);
		m.set("dispatch", &dispatch);
		m.set("logs", &logs);
		m.set("open_window", &open_window);
		m.set("register_state", &register_state);
		m.set("activate_state", &activate_state);

		global->Set(v8str("engine"), m.new_instance());
	}
}