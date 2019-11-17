#pragma once
#include <r2/bindings/bindings.h>

#include <v8.h>
#include <v8pp/convert.hpp>
#include <v8pp/class.hpp>

#include <string>
#include <vector>
using namespace std;

#define evt(eng,name,...) event(__FILE__, __LINE__, eng, name, ##__VA_ARGS__)

namespace r2 {
    class data_container;
    class r2engine;
	class var;
    class event {
        public:
            typedef struct _caller {
                _caller() {
                    file = "";
                    line = 0;
                }
                ~_caller() { }
                string file;
                int line;
            } caller;

            event(const string& file, const int line, const string& name, bool has_data = false, bool recursive = true);
			event(v8Args args);
            ~event();

            caller emitted_at() const;
            bool is_recursive() const;
            string name() const;
            data_container* data() const;

            void stop_propagating() { m_recurse = false; }

			void set_v8_local(void* local) { m_v8local = local; }
			void* v8_local() { return m_v8local; }

			void set_script_data_from_cpp(const var& v);
			void set_script_data(v8Args args);
			v8::Local<v8::Value> get_script_data() const;

        protected:
            caller m_caller;
            data_container* m_data;
			var m_scriptData;
            string m_name;
            bool m_recurse;
			void* m_v8local;
    };

    class event_receiver {
        public:
            event_receiver();
            virtual ~event_receiver();

            void add_child(event_receiver* child);
            void remove_child(event_receiver* child);

            // try not to call this directly
            virtual void handle(event* evt) = 0;

            // call this instead
            void dispatch(event* evt);

        protected:
            vector<event_receiver*> m_children;
    };
}

#define v8str(str) v8::String::NewFromUtf8(isolate, str)

namespace v8pp {
	template<>
	struct convert<r2::event> {
		using from_type = r2::event;
		using to_type = v8::Handle<v8::Object>;

		static bool is_valid(v8::Isolate* isolate, v8::Handle<v8::Value> value) {
			return !value.IsEmpty() && value->IsObject();
		}

		static r2::event from_v8(v8::Isolate* isolate, v8::Local<v8::Value> value)
		{
			if (!is_valid(isolate, value)) {
				throw invalid_argument(isolate, value, "Object");
			}
			r2::event* object = class_<r2::event, raw_ptr_traits>::unwrap_object(isolate, value);
			if (object) {
				return *object;
			}
			throw std::runtime_error("failed to unwrap C++ object");
		}

		static v8::Handle<v8::Object> to_v8(v8::Isolate* isolate, r2::event const& value) {
			v8::EscapableHandleScope scope(isolate);
			v8::Local<v8::Object> obj = v8::Object::New(isolate);

			obj->Set(v8str("name"), convert<string>::to_v8(isolate, value.name()));
			obj->Set(v8str("data"), value.get_script_data());

			return scope.Escape(obj);
		}
	};
}