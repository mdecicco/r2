#include <r2/engine.h>
#include <memory.h>

namespace r2 {
    static const char* index_names[5] = {
        nullptr, "byte", "short", nullptr, "int"
    };

    index_buffer::index_buffer(index_type type, size_t max_count) : gpu_buffer(max_count * type) {
        size_t idxSz = type;
        m_type = type;
        m_maxCount = max_count;
        m_indexCount = 0;
        r2Log("Allocating %s for a maximum of %d indices of format [%s] (buf: %d)", format_size(idxSz * max_count), max_count, index_names[type], m_id);
        m_data = new unsigned char[idxSz * max_count];
    }
    index_buffer::~index_buffer() {
        r2Log("Deallocating index buffer of format [%s]: (%s / %s | %d / %d indices used) (buf: %d)", index_names[m_type], format_size(used_size()), format_size(max_size()), m_indexCount, m_maxCount, m_id);
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
            r2Error("Insufficient space in index buffer of type [%s] for %d indices (%d max) (buf: %d)", index_names[m_type], count, m_maxCount, m_id);
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

        r2Log("Buffering data range: %zu -> %zu (buf: %d)", seg.memBegin, seg.memEnd, m_id);
        memcpy((u8*)m_data + seg.memBegin, data, seg.memEnd - seg.memBegin);
        m_indexCount += count;

		appended(seg.memBegin, seg.memEnd);

        return seg;
    }

	void index_buffer::update(const idx_bo_segment& seg, const void* data) {
		if (!seg.is_valid()) {
			r2Error("Attempted to update invalid segment of index buffer. Ignoring");
			return;
		}
		if (seg.buffer != this) {
			r2Error("Segment for another uniform buffer passed to index_buffer::update");
			return;
		}

		if (seg.memBegin > m_type * m_indexCount) {
			r2Error("Out of range segment.memBegin (%llu) provided to index_buffer::update", seg.memBegin);
			return;
		}

		if (seg.memEnd > m_type * m_indexCount) {
			r2Error("Out of range segment.memEnd (%llu) provided to index_buffer::update", seg.memEnd);
			return;
		}

		memcpy((u8*)m_data + seg.memBegin, data, seg.memsize());
		updated(seg.memBegin, seg.memEnd);
	}
}
