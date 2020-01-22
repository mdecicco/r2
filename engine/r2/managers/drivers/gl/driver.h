#pragma once
#include <r2/managers/renderman.h>
#include <r2/managers/memman.h>
#include <GL/glcorearb.h>

#define glCall(...) { glGetError(); __VA_ARGS__; printGlError(#__VA_ARGS__); }

namespace r2 {
	class gl_shader_program : public shader_program {
		public:
			gl_shader_program();
			virtual ~gl_shader_program();

			virtual bool deserialize(const unsigned char* data, size_t length);
			virtual bool serialize(unsigned char** data, size_t* length);

			virtual void activate();
			virtual void deactivate();
			virtual bool check_compatible(render_node* node);

			virtual i32 get_uniform_location(const mstring& name);
			virtual void uniform1i(u32 loc, i32 value);
			virtual void uniform2i(u32 loc, i32 v0, i32 v1);
			virtual void uniform3i(u32 loc, i32 v0, i32 v1, i32 v2);
			virtual void uniform4i(u32 loc, i32 v0, i32 v1, i32 v2, i32 v3);
			virtual void uniform1i_arr(u32 loc, u32 count, i32* values);
			virtual void uniform2i_arr(u32 loc, u32 count, i32* values);
			virtual void uniform3i_arr(u32 loc, u32 count, i32* values);
			virtual void uniform4i_arr(u32 loc, u32 count, i32* values);
			virtual void uniform1ui(u32 loc, u32 value);
			virtual void uniform2ui(u32 loc, u32 v0, u32 v1);
			virtual void uniform3ui(u32 loc, u32 v0, u32 v1, u32 v2);
			virtual void uniform4ui(u32 loc, u32 v0, u32 v1, u32 v2, u32 v3);
			virtual void uniform1ui_arr(u32 loc, u32 count, u32* values);
			virtual void uniform2ui_arr(u32 loc, u32 count, u32* values);
			virtual void uniform3ui_arr(u32 loc, u32 count, u32* values);
			virtual void uniform4ui_arr(u32 loc, u32 count, u32* values);
			virtual void uniform1f(u32 loc, f32 value);
			virtual void uniform2f(u32 loc, f32 v0, f32 v1);
			virtual void uniform3f(u32 loc, f32 v0, f32 v1, f32 v2);
			virtual void uniform4f(u32 loc, f32 v0, f32 v1, f32 v2, f32 v3);
			virtual void uniform1f_arr(u32 loc, u32 count, f32* values);
			virtual void uniform2f_arr(u32 loc, u32 count, f32* values);
			virtual void uniform3f_arr(u32 loc, u32 count, f32* values);
			virtual void uniform4f_arr(u32 loc, u32 count, f32* values);
			virtual void uniform1d(u32 loc, f64 value);
			virtual void uniform2d(u32 loc, f64 v0, f64 v1);
			virtual void uniform3d(u32 loc, f64 v0, f64 v1, f64 v2);
			virtual void uniform4d(u32 loc, f64 v0, f64 v1, f64 v2, f64 v3);
			virtual void uniform1d_arr(u32 loc, u32 count, f64* values);
			virtual void uniform2d_arr(u32 loc, u32 count, f64* values);
			virtual void uniform3d_arr(u32 loc, u32 count, f64* values);
			virtual void uniform4d_arr(u32 loc, u32 count, f64* values);
			virtual void uniform_matrix_2x2d(u32 loc, u32 count, f64* values);
			virtual void uniform_matrix_2x2f(u32 loc, u32 count, f32* values);
			virtual void uniform_matrix_2x3d(u32 loc, u32 count, f64* values);
			virtual void uniform_matrix_2x3f(u32 loc, u32 count, f32* values);
			virtual void uniform_matrix_2x4d(u32 loc, u32 count, f64* values);
			virtual void uniform_matrix_2x4f(u32 loc, u32 count, f32* values);
			virtual void uniform_matrix_3x3d(u32 loc, u32 count, f64* values);
			virtual void uniform_matrix_3x3f(u32 loc, u32 count, f32* values);
			virtual void uniform_matrix_3x2d(u32 loc, u32 count, f64* values);
			virtual void uniform_matrix_3x2f(u32 loc, u32 count, f32* values);
			virtual void uniform_matrix_3x4d(u32 loc, u32 count, f64* values);
			virtual void uniform_matrix_3x4f(u32 loc, u32 count, f32* values);
			virtual void uniform_matrix_4x4d(u32 loc, u32 count, f64* values);
			virtual void uniform_matrix_4x4f(u32 loc, u32 count, f32* values);
			virtual void uniform_matrix_4x2d(u32 loc, u32 count, f64* values);
			virtual void uniform_matrix_4x2f(u32 loc, u32 count, f32* values);
			virtual void uniform_matrix_4x3d(u32 loc, u32 count, f64* values);
			virtual void uniform_matrix_4x3f(u32 loc, u32 count, f32* values);
			virtual void texture2D(u32 loc, u32 index, texture_buffer* texture);

		protected:
			friend class gl_render_driver;
			GLuint m_program;

			struct uniform_block_info { u32 loc, bindIndex; };
			munordered_map<mstring, uniform_block_info> m_uniformBlocks;
			const uniform_block_info& block_info(uniform_block* uniforms);
	};

    class gl_render_driver : public render_driver {
        public:
			gl_render_driver(render_man* m);
            virtual ~gl_render_driver();

			virtual shader_program* load_shader(const mstring& file, const mstring& assetName);
			virtual void generate_vao(r2::render_node* node);
			virtual void free_vao(r2::render_node* node);
			virtual void bind_vao(r2::render_node* node);
			virtual void unbind_vao();
            virtual void sync_buffer(gpu_buffer* buf);
			virtual void free_buffer(gpu_buffer* buf);
			virtual void sync_texture(texture_buffer* buf);
			virtual void free_texture(texture_buffer* buf);
			virtual void present_texture(texture_buffer* buf, shader_program* shader, render_buffer* target = 0);
			virtual void sync_render_target(render_buffer* buf);
			virtual void free_render_target(render_buffer* buf);
			virtual void bind_render_target(render_buffer* buf);
			virtual void fetch_render_target_pixel(render_buffer* buf, u32 x, u32 y, size_t attachmentIdx, void* dest, size_t pixelSize);
			GLuint get_texture_id(texture_buffer* buf);
			virtual void bind_uniform_block(shader_program* shader, uniform_block* uniforms);
			virtual void clear_framebuffer(const vec4f& color, bool clearDepth);
			virtual void set_viewport(const vec2i& position, const vec2i& dimensions);

			virtual size_t get_uniform_attribute_size(uniform_format* fmt, u16 idx, uniform_attribute_type type) const;
			virtual void serialize_uniform_value(const void* input, void* output, uniform_format* fmt, u16 idx, uniform_attribute_type type) const;
			virtual size_t get_uniform_buffer_block_offset_alignment() const;
			virtual void render_node(r2::render_node* node, uniform_block* scene);


        protected:
            render_man* m_mgr;
			munordered_map<size_t, GLuint> m_buffers;
			munordered_map<size_t, GLuint> m_textures;
			munordered_map<size_t, std::pair<GLuint, GLuint>> m_targets;
			munordered_map<mstring, GLuint> m_vaos;
			render_buffer* m_target;

			GLuint m_fsqVao;
			GLuint m_fsqVbo;
    };

    class gl_draw_call : public draw_call {
        public:
			gl_draw_call();
            ~gl_draw_call();
    };

	void printGlError(const char* funcName);
};