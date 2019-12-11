#pragma once
#include <r2/managers/memman.h>
#include <r2/utilities/buffer.h>

namespace r2 {
    class r2engine;

    enum instance_attribute_type {
        iat_int = 0,
        iat_float,
        iat_uint,
        iat_vec2i,
        iat_vec2f,
        iat_vec2ui,
        iat_vec3i,
        iat_vec3f,
        iat_vec3ui,
        iat_vec4i,
        iat_vec4f,
        iat_vec4ui,
        iat_mat2i,
        iat_mat2f,
        iat_mat2ui,
        iat_mat3i,
        iat_mat3f,
        iat_mat3ui,
        iat_mat4i,
        iat_mat4f,
        iat_mat4ui
    };

    class instance_format {
        public:
            instance_format();
            instance_format(const instance_format& o);
            ~instance_format();

            void add_attr(instance_attribute_type type);
            bool operator==(const instance_format& rhs) const;
			const mvector<instance_attribute_type>& attributes() const;

            size_t size() const;
			size_t offsetOf(u16 attrIdx) const;
            mstring to_string() const;
			mstring hash_name() const;

        protected:
            mvector<instance_attribute_type> m_attrs;
            size_t m_instanceSize;
            mstring m_fmtString;
			mstring m_hashName;
    };

    class instance_buffer;
    struct ins_bo_segment : public gpu_buffer_segment {
		ins_bo_segment() : gpu_buffer_segment(), buffer(nullptr) { }

		ins_bo_segment sub(size_t _begin, size_t _end, size_t _memBegin, size_t _memEnd) {
			ins_bo_segment seg;
			seg.begin = begin + _begin;
			seg.end = begin + _end;
			seg.memBegin = memBegin + _memBegin;
			seg.memEnd = memBegin + _memEnd;
			seg.buffer = buffer;
			return seg;
		}

        instance_buffer* buffer;
    };

    class instance_buffer : public gpu_buffer {
        public:
            instance_buffer(instance_format* fmt, size_t max_count);
            ~instance_buffer();

            instance_format* format() const;
			virtual void* data() const;

            ins_bo_segment append(const void* data, size_t count);
			void update(const ins_bo_segment& segment, const void* data);

        protected:
            instance_format* m_format;
            size_t m_instanceCount;
            size_t m_maxCount;
            unsigned char* m_data;
    };
}
