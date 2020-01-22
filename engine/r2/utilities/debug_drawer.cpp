#include <r2/utilities/debug_drawer.h>
#include <r2/engine.h>

namespace r2 {
	debug_drawer::debug_drawer(scene* s, shader_program* shader, size_t max_line_vertices, size_t max_triangle_vertices) {
		m_scene = s;
		m_lineVertexCapacity = max_line_vertices;
		m_lineVertexCount = 0;
		m_triangleVertexCapacity = max_triangle_vertices;
		m_triangleVertexCount = 0;
		m_lines = nullptr;
		m_triangles = nullptr;
		m_material = nullptr;
		m_vfmt = nullptr;
		m_triangleVertices = nullptr;
		m_lineVertices = nullptr;
		m_valid = false;

		if (max_line_vertices == 0) {
			r2Error("debug_drawer must have a maximum line vertex count > 0");
			return;
		}

		if (max_triangle_vertices == 0) {
			r2Error("debug_drawer must have a maximum triangle vertex count > 0");
			return;
		}

		if (!shader) {
			r2Error("debug_drawer must have a shader that uses the vertex format { vec3f (position), vec4f (color) }");
			return;
		}

		m_vfmt = new vertex_format();
		m_vfmt->add_attr(vat_vec3f);
		m_vfmt->add_attr(vat_vec4f);

		m_material = new node_material();
		m_material->set_shader(shader);

		mesh_construction_data triangles(m_vfmt);
		triangles.set_max_vertex_count(max_triangle_vertices);
		triangles.fill();
		m_triangles = s->add_mesh(&triangles);
		m_triangles->set_vertex_count(0);
		m_triangles->set_material_instance(m_material->instantiate(s));
		m_triangles->has_transparency = true;
		m_triangles->primitives = pt_triangles;

		m_triangleVertices = new debug_vertex[max_triangle_vertices];

		mesh_construction_data lines(m_vfmt);
		lines.set_max_vertex_count(max_line_vertices);
		lines.fill();
		m_lines = s->add_mesh(&lines);
		m_lines->set_vertex_count(0);
		m_lines->set_material_instance(m_material->instantiate(s));
		m_lines->has_transparency = true;
		m_lines->primitives = pt_lines;

		m_lineVertices = new debug_vertex[max_line_vertices];

		m_valid = true;
	}

	debug_drawer::~debug_drawer() {
		if (!m_valid) return;
		if (m_lines) m_scene->remove_node(m_lines);
		if (m_triangles) m_scene->remove_node(m_triangles);
		if (m_material) delete m_material;
		if (m_vfmt) delete m_vfmt;
		if (m_triangleVertices) delete m_triangleVertices;
		if (m_lineVertices) delete m_lineVertices;
	}

	void debug_drawer::begin() {
		if (!m_valid) return;
		m_lineVertexCount = 0;
		m_triangleVertexCount = 0;
	}

	void debug_drawer::end() {
		if (!m_valid) return;
		if (m_lineVertexCount > 0) m_lines->update_vertices_raw(m_lineVertices, m_lineVertexCount);
		if (m_triangleVertexCount > 0) m_triangles->update_vertices_raw(m_triangleVertices, m_triangleVertexCount);
	}
	
	void debug_drawer::line(const vec3f& a, const vec3f& b, const vec4f& color) {
		if (m_lineVertexCapacity < m_lineVertexCount + 2) {
			r2Error("Can't draw debug line, max debug line vertex count would be exceeded");
			return;
		}

		m_lineVertices[m_lineVertexCount++] = { a, color };
		m_lineVertices[m_lineVertexCount++] = { b, color };
	}

	void debug_drawer::line(const vec3f& a, const vec3f& b, const vec4f& color_a, const vec4f& color_b) {
		if (m_lineVertexCapacity < m_lineVertexCount + 2) {
			r2Error("Can't draw debug line, max debug line vertex count would be exceeded");
			return;
		}

		m_lineVertices[m_lineVertexCount++] = { a, color_a };
		m_lineVertices[m_lineVertexCount++] = { b, color_b };
	}

	void debug_drawer::triangle(const vec3f& a, const vec3f& b, const vec3f& c, const vec4f& color) {
		if (m_triangleVertexCapacity < m_triangleVertexCount + 3) {
			r2Error("Can't draw debug triangle, max debug triangle vertex count would be exceeded");
			return;
		}

		m_triangleVertices[m_lineVertexCount++] = { a, color };
		m_triangleVertices[m_lineVertexCount++] = { b, color };
		m_triangleVertices[m_lineVertexCount++] = { c, color };
	}

	void debug_drawer::triangle(const vec3f& a, const vec3f& b, const vec3f& c, const vec4f& color_a, const vec4f& color_b, const vec4f& color_c) {
		if (m_triangleVertexCapacity < m_triangleVertexCount + 3) {
			r2Error("Can't draw debug triangle, max debug triangle vertex count would be exceeded");
			return;
		}

		m_triangleVertices[m_lineVertexCount++] = { a, color_a };
		m_triangleVertices[m_lineVertexCount++] = { b, color_b };
		m_triangleVertices[m_lineVertexCount++] = { c, color_c };
	}
};