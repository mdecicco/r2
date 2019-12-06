#pragma once

#include <v8pp/convert.hpp>
#include <v8.h>

#include <r2/managers/memman.h>

#include <stdarg.h>
#include <chrono>
using namespace std;

namespace r2 {
    class engine;
	class event;
	struct log_info {
		log_info(const mstring& type, const mstring& text, f32 time);
		f32 time;
		mstring type;
		mstring text;
	};
    class log_man {
        public:
			log_man();
			~log_man();

			void log(const mstring& type,const mstring& msg,...);
			void log(const mstring& type,const mstring& msg,va_list ap);

			mvector<log_info> lines() const { return m_lines; }

		protected:
			mvector<log_info> m_lines;
			std::chrono::high_resolution_clock::time_point m_start;
    };
}
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