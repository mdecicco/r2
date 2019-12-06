#pragma once
#include <r2/managers/memman.h>
#include <r2/utilities/buffer.h>
#include <r2/config.h>

namespace r2 {
    class r2engine;

    enum vertex_attribute_type {
        vat_int = 0,
        vat_float,
        vat_uint,
        vat_vec2i,
        vat_vec2f,
        vat_vec2ui,
        vat_vec3i,
        vat_vec3f,
        vat_vec3ui,
        vat_vec4i,
        vat_vec4f,
        vat_vec4ui,
        vat_mat2i,
        vat_mat2f,
        vat_mat2ui,
        vat_mat3i,
        vat_mat3f,
        vat_mat3ui,
        vat_mat4i,
        vat_mat4f,
        vat_mat4ui
    };

    class vertex_format {
        public:
            vertex_format();
            vertex_format(const vertex_format& o);
            ~vertex_format();

            void add_attr(vertex_attribute_type type);
            bool operator==(const vertex_format& rhs) const;
			const mvector<vertex_attribute_type>& attributes() const;

            size_t size() const;
			size_t offsetOf(u16 attrIdx) const;
            mstring to_string() const;
			mstring hash_name() const;

        protected:
            mvector<vertex_attribute_type> m_attrs;
            size_t m_vertexSize;
            mstring m_fmtString;
			mstring m_hashName;
    };

    class vertex_buffer;
    struct vtx_bo_segment : public gpu_buffer_segment {
		vtx_bo_segment() : gpu_buffer_segment(), buffer(nullptr) { }
        vertex_buffer* buffer;
    };

    class vertex_buffer : public gpu_buffer {
        public:
            vertex_buffer(vertex_format* fmt, size_t max_count);
            ~vertex_buffer();

            vertex_format* format() const;
			virtual void* data() const;

            vtx_bo_segment append(const void* data, size_t count);

        protected:
            vertex_format* m_format;
            size_t m_vertexCount;
            size_t m_maxCount;
            unsigned char* m_data;
    };
}