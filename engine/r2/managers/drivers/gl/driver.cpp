#include <r2/managers/drivers/gl/driver.h>
#include <r2/engine.h>
#include <r2/utilities/gl3w.h>

namespace r2 {
	const char* glError() noexcept {
		GLenum err = glGetError();
		switch (err) {
			// opengl 2 errors (8)
			case GL_NO_ERROR: return nullptr;
			case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
			case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
			case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
			case GL_STACK_OVERFLOW: return "GL_STACK_OVERFLOW";
			case GL_STACK_UNDERFLOW: return "GL_STACK_UNDERFLOW";
			case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
			case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION";
			default: { r2Error("Unknown GL error %d", err); return nullptr; }
		}

		return nullptr;
	}

	void printGlError(const char* funcName) {
		const char* err = glError();
		if (err) {
			r2Error("%s: %s", funcName, err);
		}
		/*
		else {
			r2Log("%s", funcName);
		}
		*/
	}

	const int attr_component_counts[] = { 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 4, 4, 4, 3, 3, 3, 4, 4, 4 };
	const GLenum attr_component_types[] = {
		GL_INT,
		GL_FLOAT,
		GL_UNSIGNED_INT,
		GL_INT,
		GL_FLOAT,
		GL_UNSIGNED_INT,
		GL_INT,
		GL_FLOAT,
		GL_UNSIGNED_INT,
		GL_INT,
		GL_FLOAT,
		GL_UNSIGNED_INT,
		GL_INT,
		GL_FLOAT,
		GL_UNSIGNED_INT,
		GL_INT,
		GL_FLOAT,
		GL_UNSIGNED_INT,
		GL_INT,
		GL_FLOAT,
		GL_UNSIGNED_INT,
	};
	const GLenum index_component_types[] = { 0, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, 0, GL_UNSIGNED_INT };
	
	
	gl_render_driver::gl_render_driver(render_man* m) : render_driver(m) {
		assert(sizeof(GLint) == sizeof(i32));
		assert(sizeof(GLuint) == sizeof(u32));
		assert(sizeof(GLfloat) == sizeof(f32));
		assert(sizeof(vec3f) == sizeof(vec3i));
		assert(sizeof(vec3ui) == sizeof(vec3i));
		assert(sizeof(vec4f) == sizeof(vec4i));
		assert(sizeof(vec4ui) == sizeof(vec4i));
		assert(sizeof(vec2f) == sizeof(f32) * 2);
		assert(sizeof(vec3f) == sizeof(f32) * 3);
		assert(sizeof(vec4f) == sizeof(f32) * 4);
		assert(sizeof(mat2f) == sizeof(f32) * 4);
		assert(sizeof(mat3f) == sizeof(f32) * 9);
		assert(sizeof(mat4f) == sizeof(f32) * 16);

		glDepthFunc(GL_LESS);
		glEnable(GL_DEPTH_TEST);
	}
	
	gl_render_driver::~gl_render_driver() {
	}

	shader_program* gl_render_driver::load_shader(const mstring& file, const mstring& assetName) {
		gl_shader_program* shader = r2engine::get()->assets()->create<gl_shader_program>(assetName);
		if (!shader->load(file)) {
			r2engine::get()->assets()->destroy(shader);
			return nullptr;
		}
		return shader;
	}
	
	void gl_render_driver::generate_vao(r2::render_node* node) {
		const vertex_format* vfmt = node->vertices().buffer->format();
		const instance_format* ifmt = nullptr;
		if (node->instances().is_valid()) ifmt = node->instances().buffer->format();

		mstring hashStr = vfmt->hash_name();
		if (ifmt) hashStr += "," + ifmt->hash_name();

		if (m_vaos.count(hashStr) > 0) return;

		GLuint vao;
		glCall(glGenVertexArrays(1, &vao));
		glCall(glBindVertexArray(vao));

		auto attribs = vfmt->attributes();
		u16 location = 0;
		for (vertex_attribute_type t : attribs) {
			GLenum type = attr_component_types[t];

			glCall(glEnableVertexAttribArray(location));
			if (type == GL_FLOAT) {
				glCall(glVertexAttribFormat(location, attr_component_counts[t], type, GL_FALSE, vfmt->offsetOf(location)));
			} else {
				glCall(glVertexAttribIFormat(location, attr_component_counts[t], type, vfmt->offsetOf(location)));
			}
			glCall(glVertexAttribBinding(location, 0));

			location++;
		}

		if (ifmt) {
			auto iattribs = ifmt->attributes();
			u16 ilocation = 0;
			u16 attribIdx = 0;
			for (instance_attribute_type t : iattribs) {
				GLenum type = attr_component_types[t];
				size_t offset = ifmt->offsetOf(attribIdx);

				if (t == iat_mat4f || t == iat_mat4i || t == iat_mat4ui) {
					for (u8 c = 0; c < 4; c++) {
						glCall(glEnableVertexAttribArray(location + ilocation));
						if (type == GL_FLOAT) {
							glCall(glVertexAttribFormat(location + ilocation, attr_component_counts[t], type, GL_FALSE, offset + (c * 16)));
						} else {
							glCall(glVertexAttribIFormat(location + ilocation, attr_component_counts[t], type, offset + (c * 16)));
						}
						glCall(glVertexAttribBinding(location + ilocation, 1));

						ilocation++;
					}
				} else if (t == iat_mat3f || t == iat_mat3i || t == iat_mat3ui) {
					for (u8 c = 0; c < 3; c++) {
						glCall(glEnableVertexAttribArray(location + ilocation));
						if (type == GL_FLOAT) {
							glCall(glVertexAttribFormat(location + ilocation, attr_component_counts[t], type, GL_FALSE, offset + (c * 12)));
						} else {
							glCall(glVertexAttribIFormat(location + ilocation, attr_component_counts[t], type, offset + (c * 12)));
						}
						glCall(glVertexAttribBinding(location + ilocation, 1));

						ilocation++;
					}
				} else {
					glCall(glEnableVertexAttribArray(location + ilocation));
					if (type == GL_FLOAT) {
						glCall(glVertexAttribFormat(location + ilocation, attr_component_counts[t], type, GL_FALSE, offset));
					} else {
						glCall(glVertexAttribIFormat(location + ilocation, attr_component_counts[t], type, offset));
					}
					glCall(glVertexAttribBinding(location + ilocation, 1));

					ilocation++;
				}

				attribIdx++;
			}
			glCall(glVertexBindingDivisor(1, 1));
		}

		glCall(glBindVertexArray(0));

		m_vaos[hashStr] = vao;
	}

	void gl_render_driver::free_vao(r2::render_node* node) {
		const vertex_format* vfmt = node->vertices().buffer->format();
		const instance_format* ifmt = nullptr;
		if (node->instances().is_valid()) ifmt = node->instances().buffer->format();

		mstring hashStr = vfmt->hash_name();
		if (ifmt) hashStr += "," + ifmt->hash_name();

		if (m_vaos.count(hashStr) == 0) return;
		GLuint vao = m_vaos[hashStr];
		glCall(glDeleteVertexArrays(1, &vao));
		m_vaos.erase(hashStr);
	}

	void gl_render_driver::bind_vao(r2::render_node* node) {
		const vertex_format* vfmt = node->vertices().buffer->format();
		const instance_format* ifmt = nullptr;
		if (node->instances().is_valid()) ifmt = node->instances().buffer->format();

		mstring hashStr = vfmt->hash_name();
		if (ifmt) hashStr += "," + ifmt->hash_name();

		if (m_vaos.count(hashStr) == 0) generate_vao(node);

		glCall(glBindVertexArray(m_vaos[hashStr]));
	}

	void gl_render_driver::unbind_vao() {
		glCall(glBindVertexArray(0));
	}

	void gl_render_driver::sync_buffer(gpu_buffer* buf) {
		auto updates = buf->updates();

		// Don't (maybe) generate the buffer until there's something in it
		if (updates.size() == 0) return;

		if (m_buffers.count(buf->id()) == 0) {
			GLuint newBuf = 0;
			glCall(glCreateBuffers(1, &newBuf));
			glCall(glNamedBufferData(newBuf, buf->max_size(), buf->data(), GL_STATIC_DRAW)); // TODO: make the last parameter configurable
			m_buffers[buf->id()] = newBuf;
			buf->clear_updates();
			return;
		}

		for (auto seg : updates) {
			glCall(glNamedBufferSubData(m_buffers[buf->id()], seg.begin, seg.end - seg.begin, (u8*)buf->data() + seg.begin));
		}
		buf->clear_updates();
	}

	void gl_render_driver::free_buffer(gpu_buffer* buf) {
		if (m_buffers.count(buf->id()) == 0) {
			r2Warn("Buffer %d was never synced, yet render_driver::free_buffer was called on it. Ignoring.", buf->id());
			return;
		}

		glCall(glDeleteBuffers(1, &m_buffers[buf->id()]));
		m_buffers.erase(buf->id());
	}

	void gl_render_driver::bind_uniform_block(shader_program* _shader, uniform_block* uniforms) {
		gl_shader_program* shader = ((gl_shader_program*)_shader);
		auto blockInfo = shader->block_info(uniforms);
		if (blockInfo.loc == GL_INVALID_INDEX) return;
		auto bufferInfo = uniforms->buffer_info();
		GLuint buffer = m_buffers[bufferInfo.buffer->id()];
		glCall(glBindBufferRange(GL_UNIFORM_BUFFER, blockInfo.bindIndex, buffer, bufferInfo.memBegin, bufferInfo.memsize()));
	}

	size_t gl_render_driver::get_uniform_attribute_size(uniform_attribute_type type) const {
		// Uniform blocks MUST use the std140 storage layout
		// in order for this function to return accurate results
		static size_t attr_sizes[21] = {
			sizeof(GLint) * 1 , sizeof(GLuint) * 1 , sizeof(GLfloat) * 1 , // scalar
			sizeof(GLint) * 2 , sizeof(GLuint) * 2 , sizeof(GLfloat) * 2 , // vec2
			sizeof(GLint) * 4 , sizeof(GLuint) * 4 , sizeof(GLfloat) * 4 , // vec3
			sizeof(GLint) * 4 , sizeof(GLuint) * 4 , sizeof(GLfloat) * 4 , // vec4
			sizeof(GLint) * 8 , sizeof(GLuint) * 8 , sizeof(GLfloat) * 8 , // mat2 (internally: vec4[2])
			sizeof(GLint) * 12, sizeof(GLuint) * 12, sizeof(GLfloat) * 12, // mat3 (internally: vec4[3])
			sizeof(GLint) * 16, sizeof(GLuint) * 16, sizeof(GLfloat) * 16  // mat4 (internally: vec4[4])
		};

		return attr_sizes[type];
	}

	void gl_render_driver::serialize_uniform_value(const void* input, void* output, uniform_attribute_type type) const {
		size_t size = get_uniform_attribute_size(type);

		switch(type) {
			case uat_int:
			case uat_uint:
			case uat_float:
			case uat_vec2i:
			case uat_vec2ui:
			case uat_vec2f:
				memcpy(output, input, size);
				return;

			case uat_vec3i:
			case uat_vec3ui:
			case uat_vec3f:
				// output is a vec4f
				memset(output, 0, size);
				memcpy(output, input, sizeof(vec3f));
				return;

			case uat_vec4i:
			case uat_vec4ui:
			case uat_vec4f:
				memcpy(output, input, size);
				return;

			case uat_mat2i:
			case uat_mat2ui:
			case uat_mat2f:
				// output is a mat2x4f
				memset(output, 0, size);
				memcpy((u8*)output + sizeof(vec4f) * 0, (u8*)input + sizeof(vec2f) * 0, sizeof(vec2f)); // mat[0][0], mat[0][1]
				memcpy((u8*)output + sizeof(vec4f) * 1, (u8*)input + sizeof(vec2f) * 1, sizeof(vec2f)); // mat[1][0], mat[1][1]
				return;

			case uat_mat3i:
			case uat_mat3ui:
			case uat_mat3f:
				// output is a mat3x4f
				memset(output, 0, size);
				memcpy((u8*)output + sizeof(vec4f) * 0, (u8*)input + sizeof(vec3f) * 0, sizeof(vec3f)); // mat[0][0], mat[0][1], mat[0][2]
				memcpy((u8*)output + sizeof(vec4f) * 1, (u8*)input + sizeof(vec3f) * 1, sizeof(vec3f)); // mat[1][0], mat[1][1], mat[1][2]
				memcpy((u8*)output + sizeof(vec4f) * 2, (u8*)input + sizeof(vec3f) * 2, sizeof(vec3f)); // mat[2][0], mat[2][1], mat[2][2]
				return;

			case uat_mat4i:
			case uat_mat4ui:
			case uat_mat4f:
				memcpy(output, input, size);
				return;
		}
	}

	size_t gl_render_driver::get_uniform_buffer_block_offset_alignment() const {
		GLint alignment = 0;
		glCall(glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &alignment));
		return alignment;
	}

	void gl_render_driver::render_node(r2::render_node* node, uniform_block* scene) {
		if (!node->material_instance()) return;
		bind_vao(node);

		auto shader = node->material_instance()->material()->shader();
		shader->activate();

		if (node->material_instance()->material()->format()->size() > 0) {
			auto uniforms = node->material_instance()->uniforms();
			bind_uniform_block(shader, uniforms);
		}

		bind_uniform_block(shader, scene);

		auto vseg = node->vertices();
		auto vbo = vseg.buffer;
		auto eseg = node->indices();
		auto ebo = eseg.buffer;
		auto iseg = node->instances();
		auto ibo = iseg.buffer;

		GLuint vbo_id = m_buffers[vbo->id()];
		GLuint ebo_id = ebo == nullptr ? 0 : m_buffers[ebo->id()];
		GLuint ibo_id = ibo == nullptr ? 0 : m_buffers[ibo->id()];

		glCall(glBindVertexBuffer(0, vbo_id, vseg.memBegin, vbo->format()->size()));

		if (ibo) {
			glCall(glBindVertexBuffer(1, ibo_id, iseg.memBegin, ibo->format()->size()));
		}

		if (ebo) {
			glCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_id));

			if (ibo) {
				glCall(glDrawElementsInstanced(GL_TRIANGLES, eseg.size(), index_component_types[ebo->type()], (void*)eseg.memBegin, iseg.size()));
			} else {
				glCall(glDrawElements(GL_TRIANGLES, eseg.size(), index_component_types[ebo->type()], (void*)eseg.memBegin));
			}
			glCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
		} else {
			if (ibo) {
				glCall(glDrawArraysInstanced(GL_TRIANGLES, vseg.begin, vseg.size(), iseg.size()));
			} else {
				glCall(glDrawArrays(GL_TRIANGLES, vseg.begin, vseg.size()));
			}
		}

		unbind_vao();
	}
};