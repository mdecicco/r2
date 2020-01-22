#pragma once
#include <r2/managers/memman.h>
#include <r2/managers/assetman.h>
#include <r2/managers/stateman.h>

namespace r2 {
    class r2engine;
    class render_man;
    class gpu_buffer;
    struct gpu_buffer_segment;
    class render_node;
	class vertex_format;
	class instance_format;
	class uniform_format;
	enum uniform_attribute_type;
	class uniform_block;
	class texture_buffer;
	class render_buffer;

	class shader_program : public asset {
		public:
			shader_program() { }
			virtual ~shader_program() { }

			virtual bool deserialize(const unsigned char* data, size_t length) = 0;
			virtual bool serialize(unsigned char** data, size_t* length) = 0;

			virtual void activate() = 0;
			virtual void deactivate() = 0;
			virtual bool check_compatible(render_node* node) = 0;

			virtual i32 get_uniform_location(const mstring& name) = 0;
			virtual void uniform1i(u32 loc, i32 value) = 0;
			virtual void uniform2i(u32 loc, i32 v0, i32 v1) = 0;
			virtual void uniform3i(u32 loc, i32 v0, i32 v1, i32 v2) = 0;
			virtual void uniform4i(u32 loc, i32 v0, i32 v1, i32 v2, i32 v3) = 0;
			virtual void uniform1i_arr(u32 loc, u32 count, i32* values) = 0;
			virtual void uniform2i_arr(u32 loc, u32 count, i32* values) = 0;
			virtual void uniform3i_arr(u32 loc, u32 count, i32* values) = 0;
			virtual void uniform4i_arr(u32 loc, u32 count, i32* values) = 0;
			virtual void uniform1ui(u32 loc, u32 value) = 0;
			virtual void uniform2ui(u32 loc, u32 v0, u32 v1) = 0;
			virtual void uniform3ui(u32 loc, u32 v0, u32 v1, u32 v2) = 0;
			virtual void uniform4ui(u32 loc, u32 v0, u32 v1, u32 v2, u32 v3) = 0;
			virtual void uniform1ui_arr(u32 loc, u32 count, u32* values) = 0;
			virtual void uniform2ui_arr(u32 loc, u32 count, u32* values) = 0;
			virtual void uniform3ui_arr(u32 loc, u32 count, u32* values) = 0;
			virtual void uniform4ui_arr(u32 loc, u32 count, u32* values) = 0;
			virtual void uniform1f(u32 loc, f32 value) = 0;
			virtual void uniform2f(u32 loc, f32 v0, f32 v1) = 0;
			virtual void uniform3f(u32 loc, f32 v0, f32 v1, f32 v2) = 0;
			virtual void uniform4f(u32 loc, f32 v0, f32 v1, f32 v2, f32 v3) = 0;
			virtual void uniform1f_arr(u32 loc, u32 count, f32* values) = 0;
			virtual void uniform2f_arr(u32 loc, u32 count, f32* values) = 0;
			virtual void uniform3f_arr(u32 loc, u32 count, f32* values) = 0;
			virtual void uniform4f_arr(u32 loc, u32 count, f32* values) = 0;
			virtual void uniform1d(u32 loc, f64 value) = 0;
			virtual void uniform2d(u32 loc, f64 v0, f64 v1) = 0;
			virtual void uniform3d(u32 loc, f64 v0, f64 v1, f64 v2) = 0;
			virtual void uniform4d(u32 loc, f64 v0, f64 v1, f64 v2, f64 v3) = 0;
			virtual void uniform1d_arr(u32 loc, u32 count, f64* values) = 0;
			virtual void uniform2d_arr(u32 loc, u32 count, f64* values) = 0;
			virtual void uniform3d_arr(u32 loc, u32 count, f64* values) = 0;
			virtual void uniform4d_arr(u32 loc, u32 count, f64* values) = 0;
			virtual void uniform_matrix_2x2d(u32 loc, u32 count, f64* values) = 0;
			virtual void uniform_matrix_2x2f(u32 loc, u32 count, f32* values) = 0;
			virtual void uniform_matrix_2x3d(u32 loc, u32 count, f64* values) = 0;
			virtual void uniform_matrix_2x3f(u32 loc, u32 count, f32* values) = 0;
			virtual void uniform_matrix_2x4d(u32 loc, u32 count, f64* values) = 0;
			virtual void uniform_matrix_2x4f(u32 loc, u32 count, f32* values) = 0;
			virtual void uniform_matrix_3x3d(u32 loc, u32 count, f64* values) = 0;
			virtual void uniform_matrix_3x3f(u32 loc, u32 count, f32* values) = 0;
			virtual void uniform_matrix_3x2d(u32 loc, u32 count, f64* values) = 0;
			virtual void uniform_matrix_3x2f(u32 loc, u32 count, f32* values) = 0;
			virtual void uniform_matrix_3x4d(u32 loc, u32 count, f64* values) = 0;
			virtual void uniform_matrix_3x4f(u32 loc, u32 count, f32* values) = 0;
			virtual void uniform_matrix_4x4d(u32 loc, u32 count, f64* values) = 0;
			virtual void uniform_matrix_4x4f(u32 loc, u32 count, f32* values) = 0;
			virtual void uniform_matrix_4x2d(u32 loc, u32 count, f64* values) = 0;
			virtual void uniform_matrix_4x2f(u32 loc, u32 count, f32* values) = 0;
			virtual void uniform_matrix_4x3d(u32 loc, u32 count, f64* values) = 0;
			virtual void uniform_matrix_4x3f(u32 loc, u32 count, f32* values) = 0;
			virtual void texture2D(u32 loc, u32 index, texture_buffer* texture) = 0;
	};

    class render_driver {
        public:
            render_driver(render_man* m);
            virtual ~render_driver();

            render_man* manager() const;

			virtual shader_program* load_shader(const mstring& file, const mstring& assetName) = 0;

			virtual void generate_vao(r2::render_node* node) { }
			virtual void free_vao(r2::render_node* node) { }
			virtual void bind_vao(r2::render_node* node) { }
			virtual void unbind_vao() { }
			virtual void sync_buffer(gpu_buffer* buf) = 0;
			virtual void free_buffer(gpu_buffer* buf) = 0;
			virtual void sync_texture(texture_buffer* buf) = 0;
			virtual void free_texture(texture_buffer* buf) = 0;
			virtual void present_texture(texture_buffer* buf, shader_program* shader, render_buffer* target = 0) = 0;
			virtual void sync_render_target(render_buffer* buf) = 0;
			virtual void free_render_target(render_buffer* buf) = 0;
			virtual void bind_render_target(render_buffer* buf) = 0;
			virtual void fetch_render_target_pixel(render_buffer* buf, u32 x, u32 y, size_t attachmentIdx, void* dest, size_t pixelSize) = 0;
			virtual void bind_uniform_block(shader_program* shader, uniform_block* uniforms) = 0;
			virtual void clear_framebuffer(const vec4f& color, bool clearDepth) = 0;
			virtual void set_viewport(const vec2i& position, const vec2i& dimensions) = 0;

			// Certain drivers will pad values in uniform blocks, so the uniform
			// buffer formats will need to account for this ahead of time
			virtual size_t get_uniform_attribute_size(uniform_format* fmt, u16 idx, uniform_attribute_type type) const;
			virtual void serialize_uniform_value(const void* input, void* output, uniform_format* fmt, u16 idx, uniform_attribute_type type) const;

			// Certain drivers require uniform blocks stored in a buffer to be aligned
			// to a specific value
			virtual size_t get_uniform_buffer_block_offset_alignment() const { return 0; };

			virtual void render_node(r2::render_node* node, uniform_block* scene) = 0;


        protected:
            render_man* m_mgr;
    };

    class draw_call {
        public:
            draw_call();
            ~draw_call();

            render_node* node;
    };

    class render_man {
        public:
            render_man();
            ~render_man();

            void set_driver(render_driver* d);
            render_driver* driver() const;

        protected:
            render_driver* m_driver;
    };
};