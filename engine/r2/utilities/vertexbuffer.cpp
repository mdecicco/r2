#include <r2/engine.h>
#include <memory.h>

namespace r2 {
    static size_t attr_sizes[21] = {
        4 , 4 , 4,
        8 , 8 , 8,
        12, 12, 12,
        16, 16, 16,
        16, 16, 16,
        36, 36, 36
    };

	static mstring attr_names[21] = {
		"int"  , "float", "uint" ,
		"vec2i", "vec2f", "vec2ui",
		"vec3i", "vec3f", "vec3ui",
		"vec4i", "vec4f", "vec4ui",
		"mat2i", "mat2f", "mat2ui",
		"mat3i", "mat3f", "mat3ui",
		"mat4i", "mat4f", "mat4ui"
	};
	static mstring attr_hash_names[21] = {
		"i"  , "f"  , "u"  ,
		"2i" , "2f" , "2u" ,
		"3i" , "3f" , "3u" ,
		"4i" , "4f" , "4u" ,
		"m2i", "m2f", "m2u",
		"m3i", "m3f", "m3u",
		"m4i", "m4f", "m4u"
	};
    vertex_format::vertex_format() {
        m_vertexSize = 0;
    }
    vertex_format::vertex_format(const vertex_format& o) {
        m_attrs = o.m_attrs;
        m_vertexSize = o.m_vertexSize;
        m_fmtString = o.m_fmtString;
		m_hashName = o.m_hashName;
    }
    vertex_format::~vertex_format() {
    }
    void vertex_format::add_attr(vertex_attribute_type type) {
        m_attrs.push_back(type);

        m_vertexSize += attr_sizes[type];
        if(m_fmtString.length() > 0) m_fmtString += ", ";
        m_fmtString += attr_names[type];
		m_hashName += attr_hash_names[type];
    }
    bool vertex_format::operator==(const vertex_format &rhs) const {
        if(rhs.m_attrs.size() != m_attrs.size()) return false;
        for(int i = 0;i < m_attrs.size();i++) {
            if(m_attrs[i] != rhs.m_attrs[i]) return false;
        }

        return true;
    }
	const mvector<vertex_attribute_type>& vertex_format::attributes() const {
		return m_attrs;
	}
    size_t vertex_format::size() const {
        return m_vertexSize;
    }
	size_t vertex_format::offsetOf(u16 attrIdx) const {
		if (attrIdx > m_attrs.size()) {
			r2Error("vertex_format::offsetOf: Attribute index %d out of range (format only has %d attributes).", attrIdx, m_attrs.size());
			return 0;
		}

		size_t offset = 0;
		for(u16 i = 0;i < attrIdx;i++) offset += attr_sizes[m_attrs[i]];

		return offset;
	}
    mstring vertex_format::to_string() const {
        return m_fmtString;
    }
	mstring vertex_format::hash_name() const {
		return m_hashName;
	}

    vertex_buffer::vertex_buffer(vertex_format* fmt, size_t max_count) : gpu_buffer(max_count * fmt->size()) {
        m_format = fmt;
        m_maxCount = max_count;
        m_vertexCount = 0;
        r2Log("Allocating %s for a maximum of %d vertices of format [%s] (buf: %d)", format_size(m_format->size() * max_count), max_count, m_format->to_string().c_str(), m_id);
        m_data = new unsigned char[m_format->size() * max_count];
    }
    vertex_buffer::~vertex_buffer() {
        r2Log("Deallocating vertex buffer: (%s / %s | %zu / %zu vertices used) (buf: %d)", format_size(used_size()), format_size(max_size()), m_vertexCount, m_maxCount, m_id);
        if(m_data) delete [] m_data;
        m_data = 0;
    }
	vertex_format* vertex_buffer::format() const {
        return m_format;
    }
	void* vertex_buffer::data() const {
		return m_data;
	}
    vtx_bo_segment vertex_buffer::append(const void *data, size_t count) {
        if(count >= m_maxCount - m_vertexCount) {
            r2Error("Insufficient space in vertex buffer of format [%s] for %d vertices (%d max) (buf: %d)", m_format->to_string().c_str(), count, m_maxCount, m_id);
            vtx_bo_segment seg;
            seg.buffer = nullptr;
            memset(&seg, 0, sizeof(vtx_bo_segment));
            return seg;
        }
        vtx_bo_segment seg;
        seg.buffer = this;
        seg.begin = m_vertexCount;
        seg.memBegin = used_size();
        seg.end = seg.begin + count;
        seg.memEnd = seg.end * m_format->size();

		r2Log("Buffering data range: %zu -> %zu (buf: %d)", seg.memBegin, seg.memEnd, m_id);
        memcpy((u8*)m_data + seg.memBegin, data, seg.memEnd - seg.memBegin);
        m_vertexCount += count;

		appended(seg.memBegin, seg.memEnd);

        return seg;
    }
}
