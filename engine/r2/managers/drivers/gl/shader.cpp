#include <r2/managers/drivers/gl/driver.h>
#include <r2/engine.h>
#include <r2/utilities/gl3w.h>

namespace r2 {
	GLuint load_and_compile_shader(const char* src, GLenum shaderType) {
		// Compile the shader
		GLuint shader = 0;
		glCall(shader = glCreateShader(shaderType));
		glCall(glShaderSource(shader, 1, &src, NULL));
		glCall(glCompileShader(shader));

		// Check the result of the compilation
		GLint test;
		glCall(glGetShaderiv(shader, GL_COMPILE_STATUS, &test));
		if (!test) {
			char log[512] = { 0 };
			glCall(glGetShaderInfoLog(shader, 512, NULL, log));
			r2Error("Shader compilation failed with this message:\n%s\n", log);
		}
		return shader;
	}

	GLuint create_program(const char* vshader, const char* fshader) {
		// Load and compile the vertex and fragment shaders
		GLuint vertexShader = load_and_compile_shader(vshader, GL_VERTEX_SHADER);
		GLuint fragmentShader = load_and_compile_shader(fshader, GL_FRAGMENT_SHADER);

		// Attach the above shader to a program
		GLuint shaderProgram = 0;
		glCall(shaderProgram = glCreateProgram());
		glCall(glAttachShader(shaderProgram, vertexShader));
		glCall(glAttachShader(shaderProgram, fragmentShader));

		// Flag the shaders for deletion
		glCall(glDeleteShader(vertexShader));
		glCall(glDeleteShader(fragmentShader));

		// Link and use the program
		glCall(glLinkProgram(shaderProgram));
		GLint test;
		glCall(glGetProgramiv(shaderProgram, GL_LINK_STATUS, &test));
		if (!test) {
			char log[512] = { 0 };
			glCall(glGetShaderInfoLog(shaderProgram, 512, NULL, log));
			r2Error("Shader linking failed with this message:\n%s\n", log);
		}

		return shaderProgram;
	}

	gl_shader_program::gl_shader_program() {
		m_program = 0;
	}

	gl_shader_program::~gl_shader_program() {
		if (m_program) glDeleteProgram(m_program);
		m_program = 0;
	}

	bool gl_shader_program::deserialize(const unsigned char* data, size_t length) {
		mstring contents;
		contents.resize(length);
		memcpy(&contents[0], data, length);

		u32 vIdx = 0;
		u32 fIdx = contents.find("// fragment");
		mstring vert = contents.substr(0, fIdx);
		mstring frag = contents.substr(fIdx);


		GLuint prog = create_program(vert.c_str(), frag.c_str());
		if (prog == 0) return false;
		if (m_program) {
			glDeleteProgram(m_program);

			for (auto it = m_uniformBlocks.begin();it != m_uniformBlocks.end();it++) {
				u32 idx = 0;
				glCall(idx = glGetUniformBlockIndex(prog, it->first.c_str()));
				if (idx == GL_INVALID_INDEX) {
					// r2Error("Failed to find uniform block \"%s\" in shader.", name.c_str());
					m_uniformBlocks[it->first] = { GL_INVALID_INDEX, GL_INVALID_INDEX };
				} else {
					u32 bindingIdx = m_uniformBlocks.size();
					glCall(glUniformBlockBinding(prog, idx, bindingIdx));
					m_uniformBlocks[it->first] = { idx, bindingIdx };
					r2Log("%s: Block binding for '%s' updated to (%d, %d)", m_name.c_str(), it->first.c_str(), idx, bindingIdx);
				}
			}
		}
		m_program = prog;
		return true;
	}

	bool gl_shader_program::serialize(unsigned char** data, size_t* length) {
		return false;
	}

	void gl_shader_program::activate() {
		glCall(glUseProgram(m_program));
	}

	void gl_shader_program::deactivate() {
		glCall(glUseProgram(0));
	}

	bool gl_shader_program::check_compatible(render_node* node) {
		return false;
	}

	const gl_shader_program::uniform_block_info& gl_shader_program::block_info(uniform_block* uniforms) {
		const mstring& name = uniforms->name();
		if (m_uniformBlocks.count(name) == 0) {
			u32 idx = 0;
			glCall(idx = glGetUniformBlockIndex(m_program, name.c_str()));
			if (idx == GL_INVALID_INDEX) {
				// r2Error("Failed to find uniform block \"%s\" in shader.", name.c_str());
				m_uniformBlocks[name] = { GL_INVALID_INDEX, GL_INVALID_INDEX };
			} else {
				u32 bindingIdx = m_uniformBlocks.size();
				glCall(glUniformBlockBinding(m_program, idx, bindingIdx));
				m_uniformBlocks[name] = { idx, bindingIdx };
			}
		}

		return m_uniformBlocks[name];
	}

	i32 gl_shader_program::get_uniform_location(const mstring& name) {
		i32 loc = 0;
		glCall(loc = glGetUniformLocation(m_program, name.c_str()));
		return loc;
	}

	void gl_shader_program::uniform1i(u32 loc, i32 value) { glCall(glUniform1i(loc, value)); }
	void gl_shader_program::uniform2i(u32 loc, i32 v0, i32 v1) { glCall(glUniform2i(loc, v0, v1)); }
	void gl_shader_program::uniform3i(u32 loc, i32 v0, i32 v1, i32 v2) { glCall(glUniform3i(loc, v0, v1, v2)); }
	void gl_shader_program::uniform4i(u32 loc, i32 v0, i32 v1, i32 v2, i32 v3) { glCall(glUniform4i(loc, v0, v1, v2, v3)); }
	void gl_shader_program::uniform1i_arr(u32 loc, u32 count, i32* values) { glCall(glUniform1iv(loc, count, values)); }
	void gl_shader_program::uniform2i_arr(u32 loc, u32 count, i32* values) { glCall(glUniform2iv(loc, count, values)); }
	void gl_shader_program::uniform3i_arr(u32 loc, u32 count, i32* values) { glCall(glUniform3iv(loc, count, values)); }
	void gl_shader_program::uniform4i_arr(u32 loc, u32 count, i32* values) { glCall(glUniform4iv(loc, count, values)); }
	void gl_shader_program::uniform1ui(u32 loc, u32 value) { glCall(glUniform1ui(loc, value)); }
	void gl_shader_program::uniform2ui(u32 loc, u32 v0, u32 v1) { glCall(glUniform2ui(loc, v0, v1)); }
	void gl_shader_program::uniform3ui(u32 loc, u32 v0, u32 v1, u32 v2) { glCall(glUniform3ui(loc, v0, v1, v2)); }
	void gl_shader_program::uniform4ui(u32 loc, u32 v0, u32 v1, u32 v2, u32 v3) { glCall(glUniform4ui(loc, v0, v1, v2, v3)); }
	void gl_shader_program::uniform1ui_arr(u32 loc, u32 count, u32* values) { glCall(glUniform1uiv(loc, count, values)); }
	void gl_shader_program::uniform2ui_arr(u32 loc, u32 count, u32* values) { glCall(glUniform2uiv(loc, count, values)); }
	void gl_shader_program::uniform3ui_arr(u32 loc, u32 count, u32* values) { glCall(glUniform3uiv(loc, count, values)); }
	void gl_shader_program::uniform4ui_arr(u32 loc, u32 count, u32* values) { glCall(glUniform4uiv(loc, count, values)); }
	void gl_shader_program::uniform1f(u32 loc, f32 value) { glCall(glUniform1f(loc, value)); }
	void gl_shader_program::uniform2f(u32 loc, f32 v0, f32 v1) { glCall(glUniform2f(loc, v0, v1)); }
	void gl_shader_program::uniform3f(u32 loc, f32 v0, f32 v1, f32 v2) { glCall(glUniform3f(loc, v0, v1, v2)); }
	void gl_shader_program::uniform4f(u32 loc, f32 v0, f32 v1, f32 v2, f32 v3) { glCall(glUniform4f(loc, v0, v1, v2, v3)); }
	void gl_shader_program::uniform1f_arr(u32 loc, u32 count, f32* values) { glCall(glUniform1fv(loc, count, values)); }
	void gl_shader_program::uniform2f_arr(u32 loc, u32 count, f32* values) { glCall(glUniform2fv(loc, count, values)); }
	void gl_shader_program::uniform3f_arr(u32 loc, u32 count, f32* values) { glCall(glUniform3fv(loc, count, values)); }
	void gl_shader_program::uniform4f_arr(u32 loc, u32 count, f32* values) { glCall(glUniform4fv(loc, count, values)); }
	void gl_shader_program::uniform1d(u32 loc, f64 value) { glCall(glUniform1d(loc, value)); }
	void gl_shader_program::uniform2d(u32 loc, f64 v0, f64 v1) { glCall(glUniform2d(loc, v0, v1)); }
	void gl_shader_program::uniform3d(u32 loc, f64 v0, f64 v1, f64 v2) { glCall(glUniform3d(loc, v0, v1, v2)); }
	void gl_shader_program::uniform4d(u32 loc, f64 v0, f64 v1, f64 v2, f64 v3) { glCall(glUniform4d(loc, v0, v1, v2, v3)); }
	void gl_shader_program::uniform1d_arr(u32 loc, u32 count, f64* values) { glCall(glUniform1dv(loc, count, values)); }
	void gl_shader_program::uniform2d_arr(u32 loc, u32 count, f64* values) { glCall(glUniform2dv(loc, count, values)); }
	void gl_shader_program::uniform3d_arr(u32 loc, u32 count, f64* values) { glCall(glUniform3dv(loc, count, values)); }
	void gl_shader_program::uniform4d_arr(u32 loc, u32 count, f64* values) { glCall(glUniform4dv(loc, count, values)); }
	void gl_shader_program::uniform_matrix_2x2d(u32 loc, u32 count, f64* values) { glCall(glUniformMatrix2dv(loc, count, GL_FALSE, values)); }
	void gl_shader_program::uniform_matrix_2x2f(u32 loc, u32 count, f32* values) { glCall(glUniformMatrix2fv(loc, count, GL_FALSE, values)); }
	void gl_shader_program::uniform_matrix_2x3d(u32 loc, u32 count, f64* values) { glCall(glUniformMatrix2x3dv(loc, count, GL_FALSE, values)); }
	void gl_shader_program::uniform_matrix_2x3f(u32 loc, u32 count, f32* values) { glCall(glUniformMatrix2x3fv(loc, count, GL_FALSE, values)); }
	void gl_shader_program::uniform_matrix_2x4d(u32 loc, u32 count, f64* values) { glCall(glUniformMatrix2x4dv(loc, count, GL_FALSE, values)); }
	void gl_shader_program::uniform_matrix_2x4f(u32 loc, u32 count, f32* values) { glCall(glUniformMatrix2x4fv(loc, count, GL_FALSE, values)); }
	void gl_shader_program::uniform_matrix_3x3d(u32 loc, u32 count, f64* values) { glCall(glUniformMatrix3dv(loc, count, GL_FALSE, values)); }
	void gl_shader_program::uniform_matrix_3x3f(u32 loc, u32 count, f32* values) { glCall(glUniformMatrix3fv(loc, count, GL_FALSE, values)); }
	void gl_shader_program::uniform_matrix_3x2d(u32 loc, u32 count, f64* values) { glCall(glUniformMatrix3x2dv(loc, count, GL_FALSE, values)); }
	void gl_shader_program::uniform_matrix_3x2f(u32 loc, u32 count, f32* values) { glCall(glUniformMatrix3x2fv(loc, count, GL_FALSE, values)); }
	void gl_shader_program::uniform_matrix_3x4d(u32 loc, u32 count, f64* values) { glCall(glUniformMatrix3x4dv(loc, count, GL_FALSE, values)); }
	void gl_shader_program::uniform_matrix_3x4f(u32 loc, u32 count, f32* values) { glCall(glUniformMatrix3x4fv(loc, count, GL_FALSE, values)); }
	void gl_shader_program::uniform_matrix_4x4d(u32 loc, u32 count, f64* values) { glCall(glUniformMatrix4dv(loc, count, GL_FALSE, values)); }
	void gl_shader_program::uniform_matrix_4x4f(u32 loc, u32 count, f32* values) { glCall(glUniformMatrix4fv(loc, count, GL_FALSE, values)); }
	void gl_shader_program::uniform_matrix_4x2d(u32 loc, u32 count, f64* values) { glCall(glUniformMatrix4x2dv(loc, count, GL_FALSE, values)); }
	void gl_shader_program::uniform_matrix_4x2f(u32 loc, u32 count, f32* values) { glCall(glUniformMatrix4x2fv(loc, count, GL_FALSE, values)); }
	void gl_shader_program::uniform_matrix_4x3d(u32 loc, u32 count, f64* values) { glCall(glUniformMatrix4x3dv(loc, count, GL_FALSE, values)); }
	void gl_shader_program::uniform_matrix_4x3f(u32 loc, u32 count, f32* values) { glCall(glUniformMatrix4x3fv(loc, count, GL_FALSE, values)); }
	void gl_shader_program::texture2D(u32 loc, u32 index, texture_buffer* texture) {
		gl_render_driver* driver = (gl_render_driver*)r2engine::renderer()->driver();
		GLuint texId = driver->get_texture_id(texture);
		if (texId == 0) return;
		glCall(glActiveTexture(GL_TEXTURE0 + index));
		glCall(glBindTexture(GL_TEXTURE_2D, texId));
		uniform1i(loc, index);
	}
};