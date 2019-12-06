#include <r2/managers/renderman.h>
#include <r2/utilities/uniformbuffer.h>

namespace r2 {
	render_driver::render_driver(render_man* m) : m_mgr(m) {
	}
	render_driver::~render_driver() {
	}

	size_t render_driver::get_uniform_attribute_size(uniform_attribute_type type) const {
		static size_t attr_sizes[21] = {
			4 , 4 , 4 , // scalar
			8 , 8 , 8 , // vec2
			12, 12, 12, // vec3
			16, 16, 16, // vec4
			16, 16, 16, // mat2
			36, 36, 36, // mat3
			64, 64, 64  // mat4
		};

		return attr_sizes[type];
	}
	void render_driver::serialize_uniform_value(const void* input, void* output, uniform_attribute_type type) const {
		memcpy(output, input, get_uniform_attribute_size(type));
	}

	render_man* render_driver::manager() const {
		return m_mgr;
	}

	draw_call::draw_call() {
	}
	draw_call::~draw_call() {
	}

	render_man::render_man() : m_driver(nullptr) {
	}
	render_man::~render_man() {
	}

	void render_man::set_driver(render_driver* d) {
		m_driver = d;

		static_uniform_formats::scene();
		static_uniform_formats::node();
	}
	render_driver* render_man::driver() const {
		return m_driver;
	}
};
