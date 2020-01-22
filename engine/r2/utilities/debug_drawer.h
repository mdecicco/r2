#pragma once
#include <r2/config.h>

namespace r2 {
	class scene;
	class render_node;
	class node_material;
	class shader_program;
	class vertex_format;

	class debug_drawer {
		public:
			#pragma pack(push, 1)
			struct debug_vertex {
				vec3f position;
				vec4f color;
			};
			#pragma pack(pop)

			debug_drawer(scene* s, shader_program* shader, size_t max_line_vertices, size_t max_triangle_vertices);
			~debug_drawer();

			void begin();
			void end();

			void line(const vec3f& a, const vec3f& b, const vec4f& color = vec4f(1.0f, 1.0f, 1.0f, 1.0f));
			void line(const vec3f& a, const vec3f& b, const vec4f& color_a, const vec4f& color_b);
			void triangle(const vec3f& a, const vec3f& b, const vec3f& c, const vec4f& color = vec4f(1.0f, 1.0f, 1.0f, 1.0f));
			void triangle(const vec3f& a, const vec3f& b, const vec3f& c, const vec4f& color_a, const vec4f& color_b, const vec4f& color_c);

		protected:
			scene* m_scene;
			node_material* m_material;
			vertex_format* m_vfmt;
			render_node* m_triangles;
			render_node* m_lines;
			bool m_valid;

			debug_vertex* m_triangleVertices;
			debug_vertex* m_lineVertices;
			size_t m_triangleVertexCapacity;
			size_t m_triangleVertexCount;
			size_t m_lineVertexCapacity;
			size_t m_lineVertexCount;
	};
};
