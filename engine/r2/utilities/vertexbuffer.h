#pragma once

#include <vector>
#include <string>
using namespace std;

#include <r2/utilities/buffer.h>

namespace r2 {
    class r2engine;

    enum vertex_attribute_type {
        vat_int = 0,
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
			const vector<vertex_attribute_type>& attributes() const;

            size_t size() const;
			size_t offsetOf(u16 attrIdx) const;
            string to_string() const;
			string hash_name() const;

        protected:
            vector<vertex_attribute_type> m_attrs;
            size_t m_vertexSize;
            string m_fmtString;
			string m_hashName;
    };

    class vertex_buffer;
    struct vtx_bo_segment : public gpu_buffer_segment{
        vertex_buffer* buffer;
    };

    class vertex_buffer : public gpu_buffer {
        public:
            vertex_buffer(const vertex_format& fmt, size_t max_count);
            ~vertex_buffer();

            const vertex_format& format() const;
			virtual void* data() const;

            vtx_bo_segment append(const void* data, size_t count);

        protected:
            vertex_format m_format;
            size_t m_vertexCount;
            size_t m_maxCount;
            unsigned char* m_data;
    };
}