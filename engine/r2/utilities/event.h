#pragma once
#include <r2/bindings/bindings.h>
#include <r2/managers/memman.h>
#include <r2/bindings/v8helpers.h>

#include <marl/mutex.h>

#define evt(name,...) event(__FILE__, __LINE__, name, ##__VA_ARGS__)

#define EVT_NAME_ACTIVATE_STATE			"~0"
#define EVT_NAME_DESTROY_ENTITY			"~1"
#define EVT_NAME_ENABLE_ENTITY_UPDATES	"~2"
#define EVT_NAME_DISABLE_ENTITY_UPDATES	"~3"
#define EVT_NAME_MOUSE_EVENT			"MouseEvent"
#define EVT_NAME_KEYBOARD_EVENT			"KeyEvent"
#define EVT_NAME_JOYSTICK_EVENT			"JoystickEvent"

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

			void set_json_from_cpp(const var& v);
			void set_json_from_str(const mstring& v);
			void set_json(v8Args args);
			v8::Local<v8::Value> get_json();

        protected:
            caller m_caller;
            data_container* m_data;
			mstring m_jsonData;
            mstring m_name;
            bool m_recurse;
			bool m_internalOnly;
    };

    class event_receiver {
        public:
            event_receiver(memory_allocator* memory = nullptr);
            event_receiver(const event_receiver& o);
            virtual ~event_receiver();

			void initialize_event_receiver();
			void destroy_event_receiver();

            void add_child(event_receiver* child);
            void remove_child(event_receiver* child);

			void subscribe(const mstring& eventName);
			void unsubscribe(const mstring& eventName);

			void frame_started();

            void dispatch(event* evt);
			void dispatchAtFrameStart(event* evt);

			// Don't call this directly
			virtual void handle(event* evt) = 0;

        private:
            void _dispatch(event* evt);

			bool m_isAtFrameStart;
			memory_allocator* m_memory;
			mvector<event*>* m_frameStartEvents;
			mvector<event*>* m_nextFrameStartEvents;
            mlist<event_receiver*>* m_children;
			mlist<mstring>* m_subscribesTo;
			event_receiver* m_parent;

            // marl suggests not using non-marl blocking functions, but
            // recursive mutexes are absolutely required in this context
            recursive_mutex m_lock;
    };
};