#include <r2/utilities/texture.h>
#include <r2/engine.h>

namespace r2 {
	const u8 type_bpc[] = { 1, 1, 2, 2, 4, 4, 4 };

	texture_buffer::texture_buffer()
		: gpu_buffer(0), m_data(nullptr), m_width(0), m_height(0),
		  m_channelCount(0), m_bytesPerChannel(0), m_minFilter(tmnf_nearest),
		  m_magFilter(tmgf_nearest), m_wrapX(tw_repeat), m_wrapY(tw_repeat),
		  m_minFilterChanged(true), m_magFilterChanged(true), m_wrapXChanged(true),
		  m_wrapYChanged(true)
	{
	}

	texture_buffer::~texture_buffer() {
		if (m_data) delete[] m_data;
		m_data = nullptr;
	}

	void texture_buffer::create(u8* data, u32 width, u32 height, u8 channels, texture_type type) {
		if (m_data) delete[] m_data;
		m_size = width * height * channels * type_bpc[type];
		m_used = m_size;
		m_data = new u8[m_size];
		memcpy(m_data, data, m_size);
		m_width = width;
		m_height = height;
		m_channelCount = channels;
		m_bytesPerChannel = type_bpc[type];
		m_type = type;
		appended(0, m_size);
	}

	void texture_buffer::create(u32 width, u32 height, u8 channels, texture_type type, bool doZeroData) {
		if (m_data) delete[] m_data;
		m_size = width * height * channels * type_bpc[type];
		m_used = m_size;
		m_data = new u8[m_size];
		if (doZeroData) memset(m_data, 0, m_size);
		m_width = width;
		m_height = height;
		m_channelCount = channels;
		m_bytesPerChannel = type_bpc[type];
		m_type = type;
		appended(0, m_size);
	}

	void texture_buffer::set_pixel(u32 x, u32 y, u8 channel, void* data) {
		u8 psz = m_channelCount * m_bytesPerChannel;
		size_t pixelOffset = ((x * u32(psz)) + (y * m_width * u32(psz)));
		size_t channelOffset = channel * m_bytesPerChannel;
		memcpy(m_data + pixelOffset + channelOffset, data, m_bytesPerChannel);
		updated(pixelOffset, pixelOffset + channelOffset);
	}

	void texture_buffer::set_pixel(u32 x, u32 y, void* data) {
		u8 psz = m_channelCount * m_bytesPerChannel;
		size_t pixelOffset = ((x * u32(psz)) + (y * m_width * u32(psz)));
		memcpy(m_data + pixelOffset, data, m_bytesPerChannel);
		updated(pixelOffset, pixelOffset + psz);
	}

	void texture_buffer::set_min_filter(texture_min_filter filter) {
		m_minFilter = filter;
		m_minFilterChanged = true;
	}

	void texture_buffer::set_mag_filter(texture_mag_filter filter) {
		m_magFilter = filter;
		m_magFilterChanged = true;
	}

	void texture_buffer::set_x_wrap_mode(texture_wrap x) {
		m_wrapX = x;
		m_wrapXChanged = true;
	}

	void texture_buffer::set_y_wrap_mode(texture_wrap y) {
		m_wrapY = y;
		m_wrapYChanged = true;
	}

	void texture_buffer::clear_mode_updates() {
		m_minFilterChanged = false;
		m_magFilterChanged = false;
		m_wrapXChanged = false;
		m_wrapYChanged = false;
	}



	render_buffer::render_buffer() : gpu_buffer(0), m_depth(rbdm_no_depth), m_depthModeChanged(true), m_wasSynced(false), m_resized(false) { }
	
	render_buffer::~render_buffer() { }

	vec2i render_buffer::dimensions() {
		if (m_attachments.size() == 0) return vec2i(0, 0);
		texture_buffer* first = *m_attachments[0];
		return vec2i(first->width(), first->height());
	}

	void render_buffer::clear_mode_updates() {
		m_depthModeChanged = false;
	}

	void render_buffer::clear_size_updates() {
		m_resized = false;
	}

	void render_buffer::raise_synced_flag() {
		m_wasSynced = true;
	}

	void render_buffer::attach(texture_buffer* tex) {
		if (m_wasSynced) {
			r2Error("Render buffer %d was already synced, No more textures can be attaced to it. Attach all textures before using the render buffer", m_id);
			return;
		}
		m_attachments.push(tex);
	}

	void render_buffer::set_depth_mode(render_buffer_depth_mode mode) {
		m_depth = mode;
		m_depthModeChanged = true;
	}
	void render_buffer::resize(const vec2i& size) {
		for (u32 a = 0;a < m_attachments.size();a++) {
			texture_buffer* attachment = *m_attachments[a];
			attachment->create(size.x, size.y, attachment->channels(), attachment->type(), true);
		}

		m_resized = true;
	}

	void render_buffer::fetch_pixel(u32 x, u32 y, size_t attachmentIdx, void* dest, size_t pixelSize) {
		r2engine::renderer()->driver()->fetch_render_target_pixel(this, x, y, attachmentIdx, dest, pixelSize);
	}

	f32 render_buffer::fetch_depth(u32 x, u32 y) {
		return r2engine::renderer()->driver()->fetch_render_target_depth(this, x, y);
	}
};
