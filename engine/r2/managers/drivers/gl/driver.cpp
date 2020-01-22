#include <r2/managers/drivers/gl/driver.h>
#include <r2/engine.h>
#include <r2/utilities/gl3w.h>
#include <r2/utilities/texture.h>

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
	const GLenum texture_types[] = { GL_BYTE, GL_UNSIGNED_BYTE, GL_SHORT, GL_UNSIGNED_SHORT, GL_INT, GL_UNSIGNED_INT, GL_FLOAT };
	const GLenum texture_formats[] = { 0, GL_RED_INTEGER, GL_RG_INTEGER, GL_RGB_INTEGER, GL_RGBA_INTEGER };
	const GLenum texture_min_filters[] = { GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR };
	const GLenum texture_mag_filters[] = { GL_NEAREST, GL_LINEAR };
	const GLenum texture_wrap_modes[] = { GL_CLAMP_TO_EDGE, GL_MIRRORED_REPEAT, GL_REPEAT };
	const GLenum render_buffer_depth_modes[] = { GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT32F, GL_NONE };
	const GLenum primitive_types[] = { GL_POINTS, GL_LINES, GL_LINE_LOOP, GL_LINE_STRIP, GL_TRIANGLES, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN, GL_QUADS, GL_QUAD_STRIP, GL_POLYGON };
	
	GLenum clientTextureFormat(u8 channels, texture_type type) {
		if (type != tt_float && type != tt_unsigned_byte) return texture_formats[channels];
		switch(channels) {
			case 1:		return GL_RED;
			case 2:		return GL_RG;
			case 3:		return GL_RGB;
			case 4:		return GL_RGBA;
			default:	return 0;
		}
		return 0;
	}

	GLenum internalTextureFormat(u8 channels, texture_type type) {
		switch(channels) {
			case 1:
				switch(type) {
					case tt_byte:			return GL_R8I;
					case tt_unsigned_byte:	return GL_RED;
					case tt_short:			return GL_R16I;
					case tt_unsigned_short: return GL_R16UI;
					case tt_int:			return GL_R32I;
					case tt_unsigned_int:	return GL_R32UI;
					case tt_float:			return GL_R32F;
					default:				return 0;
				}
			case 2:
				switch(type) {
					case tt_byte:			return GL_RG8I;
					case tt_unsigned_byte:	return GL_RG;
					case tt_short:			return GL_RG16I;
					case tt_unsigned_short: return GL_RG16UI;
					case tt_int:			return GL_RG32UI;
					case tt_unsigned_int:	return GL_RG32I;
					case tt_float:			return GL_RG32F;
					default:				return 0;
				}
			case 3:
				switch(type) {
					case tt_byte:			return GL_RGB8I;
					case tt_unsigned_byte:	return GL_RGB;
					case tt_short:			return GL_RGB16I;
					case tt_unsigned_short: return GL_RGB16UI;
					case tt_int:			return GL_RGB32UI;
					case tt_unsigned_int:	return GL_RGB32I;
					case tt_float:			return GL_RGB32F;
					default:				return 0;
				}
			case 4:
				switch(type) {
					case tt_byte:			return GL_RGBA8I;
					case tt_unsigned_byte:	return GL_RGBA;
					case tt_short:			return GL_RGBA16I;
					case tt_unsigned_short: return GL_RGBA16UI;
					case tt_int:			return GL_RGBA32UI;
					case tt_unsigned_int:	return GL_RGBA32I;
					case tt_float:			return GL_RGBA32F;
					default:				return 0;
				}
			default:						return 0;
		}

		return 0;
	}
	


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

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		glCall(glGenVertexArrays(1, &m_fsqVao));
		glCall(glBindVertexArray(m_fsqVao));

		// positions
		glCall(glEnableVertexAttribArray(0));
		glCall(glVertexAttribFormat(0, 2, GL_FLOAT, GL_FALSE, 0));
		glCall(glVertexAttribBinding(0, 0));

		// texcoords
		glCall(glEnableVertexAttribArray(1));
		glCall(glVertexAttribFormat(1, 2, GL_FLOAT, GL_FALSE, 8));
		glCall(glVertexAttribBinding(1, 0));

		glCall(glBindVertexArray(0));

		f32 verts[] = {
			-1.0f, -1.0f, 0.0f, 0.0f,
			 1.0f, -1.0f, 1.0f, 0.0f,
			 1.0f,  1.0f, 1.0f, 1.0f,
			-1.0f,  1.0f, 0.0f, 1.0f
		};
		glCall(glCreateBuffers(1, &m_fsqVbo));
		glCall(glNamedBufferData(m_fsqVbo, sizeof(f32) * 16, verts, GL_STATIC_DRAW));
	}
	
	gl_render_driver::~gl_render_driver() {
		glDeleteBuffers(1, &m_fsqVbo);
		glDeleteVertexArrays(1, &m_fsqVao);
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

	void gl_render_driver::sync_texture(texture_buffer* buf) {
		// Don't (maybe) generate the buffer until there's something in it
		if (!buf->has_updates() && !buf->has_mode_updates()) return;
		auto updates = buf->updates();

		if (m_textures.count(buf->id()) == 0) {
			GLuint newTex = 0;
			glCall(glGenTextures(1, &newTex));
			glCall(glBindTexture(GL_TEXTURE_2D, newTex));
			if (buf->has_mode_updates()) {
				if (buf->min_filter_updated()) {
					glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texture_min_filters[buf->min_filter()]));
				}
				if (buf->mag_filter_updated()) {
					glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texture_mag_filters[buf->mag_filter()]));
				}
				if (buf->wrap_x_updated()) {
					glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texture_wrap_modes[buf->wrap_x()]));
				}
				if (buf->wrap_y_updated()) {
					glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texture_wrap_modes[buf->wrap_y()]));
				}
			}
			auto clientFmt = clientTextureFormat(buf->channels(), buf->type());
			auto type = texture_types[buf->type()];
			auto driverFmt = internalTextureFormat(buf->channels(), buf->type());
			glCall(glTexImage2D(GL_TEXTURE_2D, 0, driverFmt, buf->width(), buf->height(), 0, clientFmt, type, buf->data()));
			// (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
			glCall(glBindTexture(GL_TEXTURE_2D, 0));
			m_textures[buf->id()] = newTex;
			buf->clear_updates();
			return;
		}

		// The internet said this was better than updating specific parts of the buffer... we'll see
		glCall(glBindTexture(GL_TEXTURE_2D, m_textures[buf->id()]));
		if (buf->has_mode_updates()) {
			if (buf->min_filter_updated()) {
				glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texture_min_filters[buf->min_filter()]));
			}
			if (buf->mag_filter_updated()) {
				glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texture_mag_filters[buf->mag_filter()]));
			}
			if (buf->wrap_x_updated()) {
				glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texture_wrap_modes[buf->wrap_x()]));
			}
			if (buf->wrap_y_updated()) {
				glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texture_wrap_modes[buf->wrap_y()]));
			}
			buf->clear_mode_updates();
			if (!buf->has_updates()) return;
		}

		auto clientFmt = clientTextureFormat(buf->channels(), buf->type());
		auto type = texture_types[buf->type()];
		auto driverFmt = internalTextureFormat(buf->channels(), buf->type());
		glCall(glTexImage2D(GL_TEXTURE_2D, 0, driverFmt, buf->width(), buf->height(), 0, clientFmt, type, buf->data()));
		glCall(glBindTexture(GL_TEXTURE_2D, 0));
		buf->clear_updates();
	}

	void gl_render_driver::free_texture(texture_buffer* buf) {
		if (m_textures.count(buf->id()) == 0) {
			r2Warn("Texture %d was never synced, yet render_driver::free_texture was called on it. Ignoring.", buf->id());
			return;
		}

		glCall(glDeleteTextures(1, &m_textures[buf->id()]));
		m_textures.erase(buf->id());
	}

	void gl_render_driver::present_texture(texture_buffer* buf, shader_program* shader, render_buffer* target) {
		render_buffer* currentTarget = m_target;
		bind_render_target(target);

		shader->activate();
		i32 loc = shader->get_uniform_location("tex");
		if (loc == -1) {
			r2Error("Shader \"%s\" does not have a 'tex' uniform to bind texture to, it can not be used with render_driver::present_texture", shader->name().c_str());
			shader->deactivate();
		}

		shader->texture2D(loc, 0, buf);

		glCall(glBindVertexArray(m_fsqVao));
		glCall(glBindVertexBuffer(0, m_fsqVbo, 0, sizeof(f32) * 4));
		glCall(glDrawArrays(GL_TRIANGLE_FAN, 0, 4));
		glCall(glBindVertexArray(0));
		shader->deactivate();

		bind_render_target(currentTarget);
	}

	void gl_render_driver::sync_render_target(render_buffer* buf) {
		size_t attachment_count = buf->attachment_count();
		if (attachment_count == 0) {
			r2Error("Render buffer %d has no attachments, yet render_driver::sync_render_target was called on it. Ignoring.", buf->id());
			return;
		}

		if (m_targets.count(buf->id()) == 0) {
			GLuint framebuffer = 0;
			GLuint depthbuffer = 0;
			glCall(glGenFramebuffers(1, &framebuffer));
			glCall(glBindFramebuffer(GL_FRAMEBUFFER, framebuffer));

			if (buf->depth_mode() != rbdm_no_depth) {
				GLenum mode = render_buffer_depth_modes[buf->depth_mode()];
				texture_buffer* first_attachment = buf->attachment(0);
				glCall(glGenRenderbuffers(1, &depthbuffer));
				glCall(glBindRenderbuffer(GL_RENDERBUFFER, depthbuffer));
				glCall(glRenderbufferStorage(GL_RENDERBUFFER, mode, first_attachment->width(), first_attachment->height()));
				glCall(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthbuffer));
			}

			mvector<GLenum> drawBuffers;
			for (size_t i = 0;i < attachment_count;i++) {
				texture_buffer* tex = buf->attachment(i);
				sync_texture(tex);
				GLuint textureId = m_textures[tex->id()];
				glCall(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, textureId, 0));
				drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + i);
			}

			glCall(glNamedFramebufferDrawBuffers(framebuffer, drawBuffers.size(), &drawBuffers[0]));

			GLenum status = 0;
			glCall(status = glCheckFramebufferStatus(GL_FRAMEBUFFER));
			if (status != GL_FRAMEBUFFER_COMPLETE) {
				r2Error("Render buffer %d is incomplete", buf->id());
				glCall(glBindRenderbuffer(GL_RENDERBUFFER, 0));
				glCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
				return;
			}

			m_targets[buf->id()] = pair<GLuint, GLuint>(framebuffer, depthbuffer);

			buf->clear_mode_updates();
			glCall(glBindRenderbuffer(GL_RENDERBUFFER, 0));
			glCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

			buf->raise_synced_flag();
			return;
		}

		if (buf->depth_mode_changed()) {
			pair<GLuint, GLuint>& fb = m_targets[buf->id()];
			glCall(glBindFramebuffer(GL_FRAMEBUFFER, fb.first));

			if (fb.second != 0) {
				GLenum mode = render_buffer_depth_modes[buf->depth_mode()];
				texture_buffer* first_attachment = buf->attachment(0);
				glCall(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0));
				glCall(glDeleteRenderbuffers(1, &fb.second));
				glCall(glGenRenderbuffers(1, &fb.second));
				glCall(glBindRenderbuffer(GL_RENDERBUFFER, fb.second));
				glCall(glRenderbufferStorage(GL_RENDERBUFFER, mode, first_attachment->width(), first_attachment->height()));
				glCall(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fb.second));
			}

			glCall(glBindRenderbuffer(GL_RENDERBUFFER, 0));
			glCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
		}
	}

	void gl_render_driver::free_render_target(render_buffer* buf) {
		if (m_targets.count(buf->id()) == 0) {
			r2Error("Render buffer %d was never synced, yet render_driver::free_render_target was called on it. Ignoring.", buf->id());
			return;
		}
		pair<GLuint, GLuint>& fb = m_targets[buf->id()];
		glCall(glBindFramebuffer(GL_FRAMEBUFFER, fb.first));
		glCall(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0));
		glCall(glDeleteRenderbuffers(1, &fb.second));
		glCall(glDeleteFramebuffers(1, &fb.first));
		m_targets.erase(buf->id());
	}

	void gl_render_driver::bind_render_target(render_buffer* buf) {
		if (buf == m_target) return;

		if (!buf) {
			glCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
			vec2i windowDimensions = r2engine::window()->get_size();
			glCall(glViewport(0, 0, windowDimensions.x, windowDimensions.y));
			m_target = nullptr;
			return;
		}

		if (m_targets.count(buf->id()) == 0) {
			r2Error("Render buffer %d was never synced, yet render_driver::bind_render_target was called on it. Ignoring.", buf->id());
			return;
		}

		pair<GLuint, GLuint>& fb = m_targets[buf->id()];
		vec2i d = buf->dimensions();
		glCall(glBindFramebuffer(GL_FRAMEBUFFER, fb.first));
		glCall(glViewport(0, 0, d.x, d.y));
		m_target = buf;
	}

	void gl_render_driver::fetch_render_target_pixel(render_buffer* buf, u32 x, u32 y, size_t attachmentIdx, void* dest, size_t pixelSize) {
		texture_buffer* tex = buf->attachment(attachmentIdx);
		auto clientFmt = clientTextureFormat(tex->channels(), tex->type());
		auto type = texture_types[tex->type()];

		render_buffer* currentTarget = m_target;
		bind_render_target(buf);

		glCall(glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIdx));
		glCall(glReadPixels(x, tex->height() - y, 1, 1, clientFmt, type, dest));

		bind_render_target(currentTarget);
	}

	GLuint gl_render_driver::get_texture_id(texture_buffer* buf) {
		if (m_textures.count(buf->id()) == 0) {
			r2Error("Texture %d was never synced, so it has no GL id.", buf->id());
			return 0;
		}

		return m_textures[buf->id()];
	}

	void gl_render_driver::bind_uniform_block(shader_program* _shader, uniform_block* uniforms) {
		gl_shader_program* shader = ((gl_shader_program*)_shader);
		auto blockInfo = shader->block_info(uniforms);
		if (blockInfo.loc == GL_INVALID_INDEX) return;
		auto bufferInfo = uniforms->buffer_info();
		GLuint buffer = m_buffers[bufferInfo.buffer->id()];
		glCall(glBindBufferRange(GL_UNIFORM_BUFFER, blockInfo.bindIndex, buffer, bufferInfo.memBegin, bufferInfo.memsize()));
	}

	void gl_render_driver::clear_framebuffer(const vec4f& color, bool clearDepth) {
		glCall(glClearColor(color.x, color.y, color.z, color.w));
		GLbitfield buffers = GL_COLOR_BUFFER_BIT;
		if (clearDepth) buffers |= GL_DEPTH_BUFFER_BIT;
		glCall(glClear(buffers));
	}

	void gl_render_driver::set_viewport(const vec2i& position, const vec2i& dimensions) {
		glCall(glViewport(position.x, position.y, dimensions.x, dimensions.y));
	}

	size_t gl_render_driver::get_uniform_attribute_size(uniform_format* fmt, u16 idx, uniform_attribute_type type) const {
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

		if (idx >= 2 && fmt->attrType(idx - 2) == uat_float && fmt->attrType(idx - 1) == uat_float && type == uat_float) {
			// three floats in a row. If the next isn't a float, then this float is 8 bytes wide and only occupies the first 4 bytes
			if (fmt->attributes().size() > idx + 1 && fmt->attrType(idx + 1) != uat_float) return sizeof(GLfloat) * 2;
		}

		return attr_sizes[type];
	}

	void gl_render_driver::serialize_uniform_value(const void* input, void* output, uniform_format* fmt, u16 idx, uniform_attribute_type type) const {
		size_t size = get_uniform_attribute_size(fmt, idx, type);

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

		auto material = node->material_instance();
		auto shader = material->material()->shader();
		shader->activate();

		if (material->material()->format() && material->material()->format()->size() > 0) {
			auto uniforms = material->uniforms();
			bind_uniform_block(shader, uniforms);
		}

		bind_uniform_block(shader, node->uniforms());
		bind_uniform_block(shader, scene);

		const mlist<uniform_block*>& userUniforms = node->user_uniforms();
		for (uniform_block* block : userUniforms) {
			bind_uniform_block(shader, block);
		}

		u8 texture_count = material->texture_count();
		for (u8 i = 0;i < texture_count;i++) {
			auto texture = material->texture(i);
			shader->texture2D(texture->location, i, texture->textures[texture->currentFrame]);
		}

		auto vseg = node->vertices();
		auto vbo = vseg.buffer;
		auto eseg = node->indices();
		auto ebo = eseg.buffer;
		auto iseg = node->instances();
		auto ibo = iseg.buffer;

		GLuint vbo_id = m_buffers[vbo->id()];
		GLuint ebo_id = ebo == nullptr ? 0 : m_buffers[ebo->id()];
		GLuint ibo_id = ibo == nullptr ? 0 : m_buffers[ibo->id()];
		GLenum prim_type = primitive_types[node->primitives];

		glCall(glBindVertexBuffer(0, vbo_id, vseg.memBegin, vbo->format()->size()));

		if (ibo) {
			glCall(glBindVertexBuffer(1, ibo_id, iseg.memBegin, ibo->format()->size()));
		}

		if (ebo) {
			glCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_id));

			size_t index_count = node->index_count(); // eseg.size() = capacity of the buffer
			if (ibo) {
				size_t instance_count = node->instance_count(); //iseg.size() = capacity of the buffer
				glCall(glDrawElementsInstanced(prim_type, index_count, index_component_types[ebo->type()], (void*)eseg.memBegin, instance_count));
			} else {
				glCall(glDrawElements(prim_type, index_count, index_component_types[ebo->type()], (void*)eseg.memBegin));
			}
			glCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
		} else {
			size_t vertex_count = node->vertex_count(); // vseg.size() = capacity of the buffer
			if (ibo) {
				size_t instance_count = node->instance_count(); //iseg.size() = capacity of the buffer
				glCall(glDrawArraysInstanced(prim_type, 0, vertex_count, instance_count));
			} else {
				glCall(glDrawArrays(prim_type, 0, vertex_count));
			}
		}

		unbind_vao();
	}
};