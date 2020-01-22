#pragma once
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
		idx_bo_segment() : gpu_buffer_segment(), buffer(nullptr) { }

		idx_bo_segment sub(size_t _begin, size_t _end, size_t _memBegin, size_t _memEnd) {
			idx_bo_segment seg;
			seg.begin = begin + _begin;
			seg.end = begin + _end;
			seg.memBegin = memBegin + _memBegin;
			seg.memEnd = memBegin + _memEnd;
			seg.buffer = buffer;
			return seg;
		}

        index_buffer* buffer;
    };

    class index_buffer : public gpu_buffer {
        public:
            index_buffer(index_type type, size_t max_count);
            ~index_buffer();

            index_type type() const;
			virtual void* data() const;

            idx_bo_segment append(const void* data, size_t count);
			void update(const idx_bo_segment& segment, const void* data);

        protected:
            index_type m_type;
            size_t m_indexCount;
            size_t m_maxCount;
            unsigned char* m_data;
    };
}
