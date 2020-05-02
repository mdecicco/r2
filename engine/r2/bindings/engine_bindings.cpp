#include <r2/engine.h>
#include <v8pp/class.hpp>
#include <v8pp/json.hpp>
#include <v8pp/convert.hpp>
#include <r2/systems/scripted_sys.h>
#include <r2/utilities/interpolator.hpp>

using namespace v8;
using namespace v8pp;

namespace r2 {
	void log(v8Args args) {
		r2engine* engine = r2engine::get();
		auto isolate = args.GetIsolate();
		auto context = args.GetIsolate()->GetCurrentContext();
		auto global = context->Global();

		mstring text;
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

		mstring text;
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

		mstring text;
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
		mstring v_title = "";
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

	vec2i window_size() {
		return r2engine::get()->window()->get_size();
	}

	void register_state(state* s) {
		r2engine::get()->states()->register_state(s);
	}

	void activate_state(const mstring& stateName) {
		trace t(r2engine::isolate());
		event e(t.file, t.line, EVT_NAME_ACTIVATE_STATE, true, false);
		e.data()->write_string(stateName);

		r2engine::get()->dispatchAtFrameStart(&e);
	}

	void register_system(scripted_sys* system) {
		r2engine::register_scripted_system(system);
	}

	f32 fps() {
		return r2engine::get()->fps();
	}

	size_t kb2b(size_t kb) { return KBtoB(kb); }
	size_t mb2b(size_t mb) { return MBtoB(mb); }
	size_t gb2b(size_t gb) { return GBtoB(gb); }

	void bind_engine(context* ctx) {
		bind_event(ctx);
		bind_math(ctx);
		bind_imgui(ctx);
		bind_graphics(ctx);
		bind_io(ctx);

		auto isolate = ctx->isolate();
		auto context = isolate->GetCurrentContext();
		auto global = context->Global();

		module m(isolate);

		// state, do not remove from v8pp 
		{
			class_<state, v8pp::raw_ptr_traits> s(isolate);
			s.ctor<v8Args>();
			register_class_state(s);
			s.set("set_update_frequency", &state::setUpdateFrequency);
			s.set("get_average_update_duration", &state::getAverageUpdateDuration);
			s.set("max_memory", property(&state::getMaxMemorySize));
			s.set("used_memory", property(&state::getUsedMemorySize));
			m.set("State", s);
		}

		{
			module itm(isolate);
			itm.set_const("None", interpolate::itm_none);
			itm.set_const("Linear", interpolate::itm_linear);
			itm.set_const("EaseInQuad", interpolate::itm_easeInQuad);
			itm.set_const("EaseOutQuad", interpolate::itm_easeOutQuad);
			itm.set_const("EaseInOutQuad", interpolate::itm_easeInOutQuad);
			itm.set_const("EaseInCubic", interpolate::itm_easeInCubic);
			itm.set_const("EaseOutCubic", interpolate::itm_easeOutCubic);
			itm.set_const("EaseInOutCubic", interpolate::itm_easeInOutCubic);
			itm.set_const("EaseInQuart", interpolate::itm_easeInQuart);
			itm.set_const("EaseOutQuart", interpolate::itm_easeOutQuart);
			itm.set_const("EaseInOutQuart", interpolate::itm_easeInOutQuart);
			itm.set_const("EaseInQuint", interpolate::itm_easeInQuint);
			itm.set_const("EaseOutQuint", interpolate::itm_easeOutQuint);
			itm.set_const("EaseInOutQuint", interpolate::itm_easeInOutQuint);
			m.set("Interpolation", itm);
		}

		{
			class_<scene_entity, v8pp::raw_ptr_traits> s(isolate);
			s.ctor<v8Args>();
			register_class_state(s);
			s.set("id", property(&scene_entity::id));
			s.set("name", property(&scene_entity::name));
			s.set("destroy", &scene_entity::destroy);
			s.set("set_update_frequency", &scene_entity::setUpdateFrequency);
			s.set("add_child", &scene_entity::add_child_entity);
			s.set("remove_child", &scene_entity::remove_child_entity);
			s.set("subscribe", &scene_entity::subscribe);
			s.set("unsubscribe", &scene_entity::unsubscribe);
			s.set("add_component", &scene_entity::add_custom_component);
			s.set("remove_component", &scene_entity::remove_custom_component);
			s.set("set_transition", &scene_entity::set_interpolation);
			m.set("Entity", s);
		}

		{
			class_<scripted_sys, v8pp::raw_ptr_traits> s(isolate);
			s.ctor<v8Args>();
			register_class_state(s);
			s.set("set_update_frequency", &scripted_sys::setUpdateFrequency);
			s.set("get_average_update_duration", &scripted_sys::getAverageUpdateDuration);
			s.set("query_components", &scripted_sys::queryComponents);
			m.set("System", s);
		}

		{
			class_<log_info, v8pp::raw_ptr_traits> s(isolate);
			register_class_state(s);
			s.auto_wrap_objects(true);
			s.set("text", &log_info::text);
			s.set("time", &log_info::time);
			s.set("type", &log_info::type);
			ctx->set("LogType", s);
		}

		m.set("log", &log);
		m.set("warn", &warn);
		m.set("error", &error);
		m.set("dispatch", &dispatch);
		m.set("logs", &logs);
		m.set("open_window", &open_window);
		m.set("window_size", &window_size);
		m.set("register_state", &register_state);
		m.set("activate_state", &activate_state);
		m.set("register_system", &register_system);
		m.set("frame_rate", &fps);

		module mem(isolate);
		mem.set("Kilobytes", kb2b);
		mem.set("Megabytes", mb2b);
		mem.set("Gigabytes", gb2b);
		global->Set(v8str("Memory"), mem.new_instance());

		global->Set(v8str("engine"), m.new_instance());
	}

	void release_state_objects() {
		Isolate* i = r2engine::isolate();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<scene_entity>()).remove_objects();

		release_event_objects();
		release_math_objects();
		release_imgui_objects();
		release_graphics_objects();
		release_io_objects();
	}

	void reset_state_object_storage() {
		Isolate* i = r2engine::isolate();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<scene_entity>()).reset_objects_map();

		reset_event_object_storage();
		reset_math_object_storage();
		reset_imgui_object_storage();
		reset_graphics_object_storage();
		reset_io_object_storage();
	}
}