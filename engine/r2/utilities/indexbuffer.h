#ifndef INDEX_BUFFER
#define INDEX_BUFFER

#include <vector>
#include <string>
using namespace std;

#include <r2/utilities/buffer.h>

namespace r2 {
    class r2engine;

    enum index_type {
        it_byte,
        it_short,
        it_int
    };

    class index_buffer;
    struct idx_bo_segment : public mesh_buffer_segment {
        index_buffer* buffer;
    };

    class index_buffer : public mesh_buffer {
        public:
            index_buffer(r2engine* e,index_type type,size_t max_count);
            ~index_buffer();

            index_type type() const;
            size_t used_size() const;
            size_t unused_size() const;
            size_t max_size() const;

            idx_bo_segment append(const void* data,size_t count);

        protected:
            r2engine* m_eng;
            index_type m_type;
            size_t m_indexCount;
            size_t m_maxCount;
            unsigned char* m_data;
    };
}

#endif /* end of include guard: INDEX_BUFFER */
