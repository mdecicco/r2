#include <r2/utilities/buffer.h>
#include <r2/managers/renderman.h>

namespace r2 {
    static size_t nextBufferId = 0;
	gpu_buffer::gpu_buffer(size_t max_size) : m_id(nextBufferId++), m_size(max_size), m_used(0) {
    }

	gpu_buffer::~gpu_buffer() {
		// TODO: Tell the driver
    }

	void gpu_buffer::appended(size_t begin, size_t end) {
		m_used += end - begin;
		updated(begin, end);
	}

	void gpu_buffer::updated(size_t begin, size_t end) {
		/*
		Find out if this update record can be combined with
		another to reduce calls to GPU driver code
		*/

		for (auto& seg : m_updates) {
			if (seg.end == begin) { seg.end = end; return; }
			if (seg.begin == end) { seg.begin = begin; return; }
		}

		// It can't
		m_updates.push_back({ begin, end });
	}

	size_t gpu_buffer::id() const {
		return m_id;
	}

	bool gpu_buffer::has_updates() const { 
		return m_updates.size() > 0;
	}

	const std::list<gpu_buffer::changed_buffer_segment>& gpu_buffer::updates() const {
		return m_updates;
	}

	void gpu_buffer::clear_updates() {
		m_updates.clear();
	}

	size_t gpu_buffer::used_size() const {
		return m_used;
	}
	size_t gpu_buffer::unused_size() const {
		return m_size - m_used;
	}
	size_t gpu_buffer::max_size() const {
		return m_size;
	}


	buffer_pool::buffer_pool() {
	}

	buffer_pool::~buffer_pool() {
	}

	void buffer_pool::sync_buffers(render_driver* driver) {
		for (auto buf : m_buffers) driver->sync_buffer(buf);
	}
}
