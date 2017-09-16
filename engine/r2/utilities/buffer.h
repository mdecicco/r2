#ifndef BUFFER_BASE
#define BUFFER_BASE

#include <stddef.h>

namespace r2 {
    struct mesh_buffer_segment {
        size_t begin;
        size_t end;
        size_t memBegin;
        size_t memEnd;

        size_t size() const { return end - begin; }
        size_t memsize() const { return memEnd - memBegin; }
        bool is_valid() const { return size() != 0; }
    };
    
    class mesh_buffer {
        public:
            mesh_buffer();
            virtual ~mesh_buffer();

            size_t id() const;
        protected:
            size_t m_id;
    };
}

#endif /* end of include guard: BUFFER_BASE */
