#ifndef RENDER_MANAGER
#define RENDER_MANAGER

#include <vector>
using namespace std;

namespace r2 {
    class r2engine;
    class render_man;
    class mesh_buffer;
    struct mesh_buffer_segment;
    class render_node;

    class render_driver {
        public:
            render_driver(render_man* m);
            virtual ~render_driver();

            render_man* manager() const;

            virtual void load_buffer(mesh_buffer* buf) = 0;
            virtual void update_buffer(mesh_buffer* buf,mesh_buffer_segment* range,void* data) = 0;


        protected:
            render_man* m_mgr;
    };

    class draw_call {
        public:
            draw_call();
            ~draw_call();

            render_node* node;
    };

    class render_man {
        public:
            render_man(r2engine* e);
            ~render_man();

            void set_driver(render_driver* d);
            render_driver* driver() const;

        protected:
            r2engine* m_eng;
            render_driver* m_driver;
    };
}

#endif /* end of include guard: RENDER_MANAGER */
