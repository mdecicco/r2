#pragma once

#include <vector>
#include <string>
using namespace std;

#include <r2/utilities/buffer.h>

namespace r2 {
    class r2engine;

    enum index_type {
        it_unsigned_byte = 1,
        it_unsigned_short = 2,
        it_unsigned_int = 4
    };

    class index_buffer;
    struct idx_bo_segment : public gpu_buffer_segment {
        index_buffer* buffer;
    };

    class index_buffer : public gpu_buffer {
        public:
            index_buffer(index_type type, size_t max_count);
            ~index_buffer();

            index_type type() const;
			virtual void* data() const;

            idx_bo_segment append(const void* data, size_t count);

        protected:
            index_type m_type;
            size_t m_indexCount;
            size_t m_maxCount;
            unsigned char* m_data;
    };
}
