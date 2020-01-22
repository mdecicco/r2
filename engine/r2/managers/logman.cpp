#include <r2/engine.h>
#include <stdarg.h>
#include <memory.h>
#include <r2/bindings/v8helpers.h>
#include <rlutilh.h>

#include <v8pp/convert.hpp>
#include <v8.h>
#define v8str(str) v8::String::NewFromUtf8(isolate, str)

namespace v8pp {
	template<>
	struct convert<r2::log_info> {
		using from_type = r2::log_info;
		using to_type = v8::Handle<v8::Object>;

		static bool is_valid(v8::Isolate* isolate, v8::Handle<v8::Value> value) {
			return !value.IsEmpty() && value->IsObject();
		}

		static r2::log_info from_v8(v8::Isolate* isolate, v8::Handle<v8::Object> obj) {
			r2::log_info result("", "", 0.0f);

			result.time = convert<float>::from_v8(isolate, obj->Get(isolate->GetCurrentContext(), v8str("time")).ToLocalChecked());
			result.text = convert<r2::mstring>::from_v8(isolate, obj->Get(isolate->GetCurrentContext(), v8str("text")).ToLocalChecked());
			result.type = convert<r2::mstring>::from_v8(isolate, obj->Get(isolate->GetCurrentContext(), v8str("type")).ToLocalChecked());

			return result;
		}

		static v8::Handle<v8::Object> to_v8(v8::Isolate* isolate, r2::log_info const& value) {
			v8::EscapableHandleScope scope(isolate);
			v8::Local<v8::Object> obj = v8::Object::New(isolate);

			obj->Set(v8str("time"), convert<float>::to_v8(isolate, value.time));
			obj->Set(v8str("text"), convert<r2::mstring>::to_v8(isolate, value.text));
			obj->Set(v8str("type"), convert<r2::mstring>::to_v8(isolate, value.type));

			return scope.Escape(obj);
		}
	};
};

namespace r2 {
	log_info::log_info(const mstring& _type, const mstring& _text, f32 _time) : type(_type), text(_text), time(_time) { }

	log_man::log_man() : m_start(std::chrono::steady_clock::now()) {
	}

	log_man::~log_man() {
	}

	void log_man::log(const mstring& type, const mstring& msg,...) {
		va_list l;
		va_start(l, msg);
		log(type, msg, l);
		va_end(l);
	}

	void log_man::log(const mstring& type, const mstring& msg, va_list ap) {
		memory_man::push_current(memory_man::global());
		switch(type[0]) {
			case 'd':
				rlutil::setColor(rlutil::GREY);
				break;
			case 'w':
				rlutil::setColor(rlutil::YELLOW);
				break;
			case 'e':
				rlutil::setColor(rlutil::LIGHTRED);
				break;
		}

		char buf[1024] = { 0 };
		vsnprintf(buf, 1024, msg.c_str(), ap);
		mstring s = mstring(buf) + '\n';
		const char* str = s.c_str();
		RLUTIL_PRINT(str);

		std::chrono::duration<f32> dt = std::chrono::high_resolution_clock::now() - m_start;
		m_lines.push_back(log_info(type, buf, dt.count()));
		rlutil::setColor(rlutil::WHITE);
		memory_man::pop_current();

		r2engine* eng = r2engine::get();
		if (eng && eng->scripts()) {
			if (type == "error") {
				v8::Isolate* isolate = eng->scripts()->context()->isolate();
				if (isolate) isolate->ThrowException(v8str(""));
			}
			event e(__FILE__, __LINE__, "log", false, true);
			e.set_json_from_cpp(var(eng->scripts()->context()->isolate(), m_lines[m_lines.size() - 1]));
			r2engine::get()->dispatch(&e);
		}
	}
}
