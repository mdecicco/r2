#ifndef EVENT_DEF
#define EVENT_DEF

#include <string>
#include <vector>
using namespace std;

#define evt(eng,name,...) event(__FILE__,__LINE__,eng,name,##__VA_ARGS__)

namespace r2 {
    class data_container;
    class r2engine;
    class event {
        public:
            typedef struct _caller {
                _caller() {
                    file = nullptr;
                    line = 0;
                }
                ~_caller() { }
                const char* file;
                int line;
            } caller;

            event(const char* file,const int line,r2engine* eng,const string& name,bool data_storage = true,bool recursive = true);
            ~event();

            caller emitted_at() const;
            bool is_recursive() const;
            string name() const;
            data_container* data() const;

            void stop_propagating();

        protected:
            caller m_caller;
            data_container* m_data;
            r2engine* m_eng;
            string m_name;
            bool m_recurse;
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

#endif /* end of include guard: EVENT_DEF */
