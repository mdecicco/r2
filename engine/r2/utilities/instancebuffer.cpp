#include <r2/engine.h>
#include <memory.h>

namespace r2 {
    static size_t instance_attr_sizes[21] = {
        4 , 4 , 1,
        8 , 8 , 2,
        12, 12, 3,
        16, 16, 4,
        16, 16, 4,
        36, 36, 9,

    };

    static string instance_attr_names[21] = {
        "int"  , "float", "byte" ,
        "vec2i", "vec2f", "vec2b",
        "vec3i", "vec3f", "vec3b",
        "vec4i", "vec4f", "vec4b",
        "mat2i", "mat2f", "mat2b",
        "mat3i", "mat3f", "mat3b",
        "mat4i", "mat4f", "mat4b",
    };
    instance_format::instance_format() {
        m_instanceSize = 0;
    }
    instance_format::instance_format(const instance_format& o) {
        m_attrs = o.m_attrs;
        m_instanceSize = o.m_instanceSize;
        m_fmtString = o.m_fmtString;
    }
    instance_format::~instance_format() {
    }
    void instance_format::add_attr(instance_attribute_type type) {
        m_attrs.push_back(type);

        m_instanceSize += instance_attr_sizes[type];
        if(m_fmtString.length() > 0) m_fmtString += ", ";
        m_fmtString += instance_attr_names[type];
    }
    bool instance_format::operator==(const instance_format &rhs) const {
        if(rhs.m_attrs.size() != m_attrs.size()) return false;
        for(int i = 0;i < m_attrs.size();i++) {
            if(m_attrs[i] != rhs.m_attrs[i]) return false;
        }

        return true;
    }
    size_t instance_format::size() const {
        return m_instanceSize;
    }
    string instance_format::to_string() const {
        return m_fmtString;
    }

    instance_buffer::instance_buffer(r2engine* e,const instance_format& fmt,size_t max_count) {
        m_eng = e;
        m_format = fmt;
        m_maxCount = max_count;
        m_instanceCount = 0;
        r2Log("Allocating %d bytes for a maximum of %d instances of format [%s]",m_format.size() * max_count,max_count,m_format.to_string().c_str());
        m_data = new unsigned char[fmt.size() * max_count];
    }
    instance_buffer::~instance_buffer() {
        r2Log("Deallocating instance buffer of format [%s]: (%d bytes / %d bytes | %d instances / %d instances used)",m_format.to_string().c_str(),used_size(),max_size(),m_instanceCount,m_maxCount);
        if(m_data) delete [] m_data;
        m_data = 0;
    }
    instance_format instance_buffer::format() const {
        return m_format;
    }
    size_t instance_buffer::used_size() const {
        return m_instanceCount * m_format.size();
    }
    size_t instance_buffer::unused_size() const {
        return max_size() - used_size();
    }
    size_t instance_buffer::max_size() const {
        return m_maxCount * m_format.size();
    }
    ins_bo_segment instance_buffer::append(const void *data, size_t count) {
        if(count >= m_maxCount - m_instanceCount) {
            r2Error("Insufficient space in instance buffer of format [%s] for %d instances (%d max)",m_format.to_string().c_str(),count,m_maxCount);
            ins_bo_segment seg;
            seg.buffer = nullptr;
            memset(&seg,0,sizeof(ins_bo_segment));
            return seg;
        }
        ins_bo_segment seg;
        seg.buffer = this;
        seg.begin = m_instanceCount;
        seg.memBegin = used_size();
        seg.end = seg.begin + count;
        seg.memEnd = seg.end * m_format.size();

        r2Log("Buffering data range: %zu -> %zu (buf: 0x%X)",seg.memBegin,seg.memEnd,(intptr_t)m_data);
        memcpy(m_data,data,seg.memEnd - seg.memBegin);
        m_instanceCount += count;

        return seg;
    }
}
