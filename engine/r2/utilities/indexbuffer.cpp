#include <r2/engine.h>
#include <memory.h>

namespace r2 {
    static size_t index_sizes[3] = {
        1, 2, 4
    };

    static string index_names[3] = {
        "byte", "short", "int"
    };

    index_buffer::index_buffer(r2engine* e,index_type type,size_t max_count) {
        size_t idxSz = index_sizes[type];
        m_eng = e;
        m_type = type;
        m_maxCount = max_count;
        m_indexCount = 0;
        r2Log("Allocating %d bytes for a maximum of %d indices of format [%s]",idxSz * max_count,max_count,index_names[type].c_str());
        m_data = new unsigned char[idxSz * max_count];
    }
    index_buffer::~index_buffer() {
        r2Log("Deallocating index buffer of format [%s]: (%d bytes / %d bytes | %d indices / %d indices used)",index_names[m_type].c_str(),used_size(),max_size(),m_indexCount,m_maxCount);
        if(m_data) delete [] m_data;
        m_data = 0;
    }
    index_type index_buffer::type() const {
        return m_type;
    }
    size_t index_buffer::used_size() const {
        return m_indexCount * index_sizes[m_type];
    }
    size_t index_buffer::unused_size() const {
        return max_size() - used_size();
    }
    size_t index_buffer::max_size() const {
        return m_maxCount * index_sizes[m_type];
    }
    idx_bo_segment index_buffer::append(const void *data, size_t count) {
        if(count >= m_maxCount - m_indexCount) {
            r2Error("Insufficient space in index buffer of type [%s] for %d indices (%d max)",index_names[m_type].c_str(),count,m_maxCount);
            idx_bo_segment seg;
            seg.buffer = nullptr;
            memset(&seg,0,sizeof(vtx_bo_segment));
            return seg;
        }
        idx_bo_segment seg;
        seg.buffer = this;
        seg.begin = m_indexCount;
        seg.memBegin = used_size();
        seg.end = seg.begin + count;
        seg.memEnd = seg.end * index_sizes[m_type];

        r2Log("Buffering data range: %zu -> %zu (buf: 0x%X)",seg.memBegin,seg.memEnd,(intptr_t)m_data);
        memcpy(m_data,data,seg.memEnd - seg.memBegin);
        m_indexCount += count;

        return seg;
    }
}
