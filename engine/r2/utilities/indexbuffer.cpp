#include <r2/engine.h>
#include <memory.h>

namespace r2 {
    static const char* index_names[4] = {
        "byte", "short", nullptr, "int"
    };

    index_buffer::index_buffer(index_type type, size_t max_count) : gpu_buffer(max_count * type) {
        size_t idxSz = type;
        m_type = type;
        m_maxCount = max_count;
        m_indexCount = 0;
        r2Log("Allocating %d bytes for a maximum of %d indices of format [%s]", idxSz * max_count,max_count, index_names[type]);
        m_data = new unsigned char[idxSz * max_count];
    }
    index_buffer::~index_buffer() {
        r2Log("Deallocating index buffer of format [%s]: (%d bytes / %d bytes | %d indices / %d indices used)", index_names[m_type], used_size(), max_size(), m_indexCount, m_maxCount);
        if(m_data) delete [] m_data;
        m_data = 0;
    }
    index_type index_buffer::type() const {
        return m_type;
    }
	void* index_buffer::data() const {
		return m_data;
	}
    idx_bo_segment index_buffer::append(const void *data, size_t count) {
        if(count >= m_maxCount - m_indexCount) {
            r2Error("Insufficient space in index buffer of type [%s] for %d indices (%d max)", index_names[m_type], count, m_maxCount);
            idx_bo_segment seg;
            seg.buffer = nullptr;
            memset(&seg, 0, sizeof(vtx_bo_segment));
            return seg;
        }

        idx_bo_segment seg;
        seg.buffer = this;
        seg.begin = m_indexCount;
        seg.memBegin = used_size();
        seg.end = seg.begin + count;
        seg.memEnd = seg.end * m_type;

        r2Log("Buffering data range: %zu -> %zu (buf: 0x%X)", seg.memBegin, seg.memEnd,(intptr_t)m_data);
        memcpy((u8*)m_data + seg.memBegin, data, seg.memEnd - seg.memBegin);
        m_indexCount += count;

		appended(seg.memBegin, seg.memEnd);

        return seg;
    }
}
