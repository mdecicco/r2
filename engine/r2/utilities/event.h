#pragma once
#include <r2/bindings/bindings.h>
#include <r2/managers/memman.h>

#include <r2/bindings/v8helpers.h>

#define evt(name,...) event(__FILE__, __LINE__, name, ##__VA_ARGS__)

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
                mstring file;
                int line;
            } caller;

            event(const mstring& file, const int line, const mstring& name, bool has_data = false, bool recursive = true);
			event(const event& o);
			event(v8Args args);
            ~event();

            caller emitted_at() const;
            bool is_recursive() const;
            mstring name() const;
            data_container* data() const;
			bool is_internal_only() const { return m_internalOnly; }

            void stop_propagating() { m_recurse = false; }

			void set_script_data_from_cpp(const var& v);
			void set_script_data(v8Args args);
			v8::Local<v8::Value> get_script_data();

        protected:
            caller m_caller;
            data_container* m_data;
			var m_scriptData;
            mstring m_name;
            bool m_recurse;
			bool m_internalOnly;
    };

    class event_receiver {
        public:
            event_receiver(memory_allocator* memory = nullptr);
            virtual ~event_receiver();

            void add_child(event_receiver* child);
            void remove_child(event_receiver* child);

			void frame_started();

            void dispatch(event* evt);
			void dispatchAtFrameStart(event* evt);

			// Don't call this directly
			virtual void handle(event* evt) = 0;

        private:
			bool m_isAtFrameStart;
			memory_allocator* m_memory;
			mvector<event*> m_frameStartEvents;
			mvector<event*> m_nextFrameStartEvents;
            mvector<event_receiver*> m_children;
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
			r2::event* object = class_<r2::event, v8pp::raw_ptr_traits>::unwrap_object(isolate, value);
			if (object) {
				return *object;
			}
			throw std::runtime_error("failed to unwrap C++ object");
		}

		static v8::Handle<v8::Object> to_v8(v8::Isolate* isolate, r2::event const& value) {
			v8::EscapableHandleScope scope(isolate);
			v8::Local<v8::Object> obj = v8::Object::New(isolate);

			obj->Set(v8str("name"), convert<r2::mstring>::to_v8(isolate, value.name()));
			obj->Set(v8str("data"), const_cast<r2::event&>(value).get_script_data());

			return scope.Escape(obj);
		}
	};
}