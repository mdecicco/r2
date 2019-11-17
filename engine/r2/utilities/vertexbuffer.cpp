#include <r2/engine.h>
#include <memory.h>

namespace r2 {
    static size_t attr_sizes[21] = {
        4 , 4 , 1,
        8 , 8 , 2,
        12, 12, 3,
        16, 16, 4,
        16, 16, 4,
        36, 36, 9,
    };

	static string attr_names[21] = {
		"int"  , "float", "byte" ,
		"vec2i", "vec2f", "vec2b",
		"vec3i", "vec3f", "vec3b",
		"vec4i", "vec4f", "vec4b",
		"mat2i", "mat2f", "mat2b",
		"mat3i", "mat3f", "mat3b",
		"mat4i", "mat4f", "mat4b",
	};
	static string attr_hash_names[21] = {
		"i"  , "f"  , "b"  ,
		"2i" , "2f" , "2b" ,
		"3i" , "3f" , "3b" ,
		"4i" , "4f" , "4b" ,
		"m2i", "m2f", "m2b",
		"m3i", "m3f", "m3b",
		"m4i", "m4f", "m4b",
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
	const vector<vertex_attribute_type>& vertex_format::attributes() const {
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
    string vertex_format::to_string() const {
        return m_fmtString;
    }
	string vertex_format::hash_name() const {
		return m_hashName;
	}

    vertex_buffer::vertex_buffer(const vertex_format& fmt, size_t max_count) : gpu_buffer(max_count * fmt.size()) {
        m_format = fmt;
        m_maxCount = max_count;
        m_vertexCount = 0;
        r2Log("Allocating %d bytes for a maximum of %d vertices of format [%s]", m_format.size() * max_count, max_count, m_format.to_string().c_str());
        m_data = new unsigned char[m_format.size() * max_count];
    }
    vertex_buffer::~vertex_buffer() {
        r2Log("Deallocating vertex buffer of format [%s]: (%zu bytes / %zu bytes | %zu vertices / %zu vertices used)", m_format.to_string().c_str(), used_size(), max_size(), m_vertexCount, m_maxCount);
        if(m_data) delete [] m_data;
        m_data = 0;
    }
	const vertex_format& vertex_buffer::format() const {
        return m_format;
    }
	void* vertex_buffer::data() const {
		return m_data;
	}
    vtx_bo_segment vertex_buffer::append(const void *data, size_t count) {
        if(count >= m_maxCount - m_vertexCount) {
            r2Error("Insufficient space in vertex buffer of format [%s] for %d vertices (%d max)", m_format.to_string().c_str(), count, m_maxCount);
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
        seg.memEnd = seg.end * m_format.size();

        r2Log("Buffering data range: %zu -> %zu (buf: 0x%X)", seg.memBegin, seg.memEnd,(intptr_t)m_data);
        memcpy((u8*)m_data + seg.memBegin, data, seg.memEnd - seg.memBegin);
        m_vertexCount += count;

		appended(seg.memBegin, seg.memEnd);

        return seg;
    }
}
