#ifndef INSTANCE_BUFFER
#define INSTANCE_BUFFER


#include <vector>
#include <string>
using namespace std;

#include <r2/utilities/buffer.h>

namespace r2 {
    class r2engine;

    enum instance_attribute_type {
        iat_int,
        iat_float,
        iat_byte,
        iat_vec2i,
        iat_vec2f,
        iat_vec2b,
        iat_vec3i,
        iat_vec3f,
        iat_vec3b,
        iat_vec4i,
        iat_vec4f,
        iat_vec4b,
        iat_mat2i,
        iat_mat2f,
        iat_mat2b,
        iat_mat3i,
        iat_mat3f,
        iat_mat3b,
        iat_mat4i,
        iat_mat4f,
        iat_mat4b
    };

    class instance_format {
        public:
            instance_format();
            instance_format(const instance_format& o);
            ~instance_format();

            void add_attr(instance_attribute_type type);
            bool operator==(const instance_format& rhs) const;

            size_t size() const;
            string to_string() const;

        protected:
            vector<instance_attribute_type> m_attrs;
            size_t m_instanceSize;
            string m_fmtString;
    };

    class instance_buffer;
    struct ins_bo_segment : public mesh_buffer_segment {
        instance_buffer* buffer;
    };

    class instance_buffer : public mesh_buffer {
        public:
            instance_buffer(r2engine* e,const instance_format& fmt,size_t max_count);
            ~instance_buffer();

            instance_format format() const;
            size_t used_size() const;
            size_t unused_size() const;
            size_t max_size() const;

            ins_bo_segment append(const void* data,size_t count);

        protected:
            r2engine* m_eng;
            instance_format m_format;
            size_t m_instanceCount;
            size_t m_maxCount;
            unsigned char* m_data;
    };
}

#endif /* end of include guard: INSTANCE_BUFFER */
