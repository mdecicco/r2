#ifndef VERTEX_BUFFER
#define VERTEX_BUFFER

#include <vector>
#include <string>
using namespace std;

#include <r2/utilities/buffer.h>

namespace r2 {
    class r2engine;

    enum vertex_attribute_type {
        vvat_int,
        vat_float,
        vat_byte,
        vat_vec2i,
        vat_vec2f,
        vat_vec2b,
        vat_vec3i,
        vat_vec3f,
        vat_vec3b,
        vat_vec4i,
        vat_vec4f,
        vat_vec4b,
        vat_mat2i,
        vat_mat2f,
        vat_mat2b,
        vat_mat3i,
        vat_mat3f,
        vat_mat3b,
        vat_mat4i,
        vat_mat4f,
        vat_mat4b
    };

    class vertex_format {
        public:
            vertex_format();
            vertex_format(const vertex_format& o);
            ~vertex_format();

            void add_attr(vertex_attribute_type type);
            bool operator==(const vertex_format& rhs) const;

            size_t size() const;
            string to_string() const;

        protected:
            vector<vertex_attribute_type> m_attrs;
            size_t m_vertexSize;
            string m_fmtString;
    };

    class vertex_buffer;
    struct vtx_bo_segment : public mesh_buffer_segment{
        vertex_buffer* buffer;
    };

    class vertex_buffer : public mesh_buffer {
        public:
            vertex_buffer(r2engine* e,const vertex_format& fmt,size_t max_count);
            ~vertex_buffer();

            vertex_format format() const;
            size_t used_size() const;
            size_t unused_size() const;
            size_t max_size() const;

            vtx_bo_segment append(const void* data,size_t count);

        protected:
            r2engine* m_eng;
            vertex_format m_format;
            size_t m_vertexCount;
            size_t m_maxCount;
            unsigned char* m_data;
    };
}

#endif /* end of include guard: VERTEX_BUFFER */
