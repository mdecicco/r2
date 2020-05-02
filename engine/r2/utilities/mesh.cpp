#include <r2/engine.h>
#include <memory.h>

namespace r2 {
	void assign_memory(void* base, void* assignTo, size_t idx, size_t element_size) {
		memcpy((u8*)base + (idx * element_size), assignTo, element_size);
	}

	void resize_memory(void** old, size_t existing_count, size_t new_count, size_t element_size) {
		u8* new_data = new u8[new_count * element_size];
		memset(new_data, 0, new_count * element_size);
		if (*old) {
			memcpy(new_data, *old, existing_count * element_size);
			delete [] *old;
		}
		*old = new_data;
	}

	mesh_construction_data::mesh_construction_data(vertex_format* vtx_fmt, index_type idx_typ, instance_format* ins_fmt)  {
		m_vertexFormat = vtx_fmt;
		m_indexType = idx_typ;
		m_instanceFormat = ins_fmt;
		m_vertices = m_indices = m_instances = nullptr;
		m_last_vertex_idx = m_last_index_idx = m_last_instance_idx = 0;
		m_vertex_count = m_index_count = m_instance_count = 0;
		m_wasSentToGpu = false;
	}
	
	mesh_construction_data::~mesh_construction_data() {
		if (m_vertices) delete [] m_vertices;
		if (m_indices) delete [] m_indices;
		if (m_instances) delete [] m_instances;
	}

	void mesh_construction_data::set_max_vertex_count(u32 count) {
		if (count == m_vertex_count) return;
		if (!m_vertexFormat) {
			r2Error("Mesh does not have an vertex format set, you can't allocate storage for vertices for it");
			return;
		}

		if (count < m_last_vertex_idx) {
			r2Error("Setting mesh max vertex count to %d would destroy vertex data, there are currently %d vertices in this mesh", count, m_last_vertex_idx);
			return;
		}
		
		resize_memory((void**)&m_vertices, m_last_vertex_idx, count, m_vertexFormat->size());
		m_vertex_count = count;
	}

	void mesh_construction_data::set_max_index_count(u32 count) {
		if (count == m_index_count) return;

		if (count < m_last_index_idx) {
			r2Error("Setting mesh max index count to %d would destroy index data, there are currently %d indices in this mesh", count, m_last_index_idx);
			return;
		}

		resize_memory((void**)&m_indices, m_last_index_idx, count, m_indexType);
		m_index_count = count;
	}

	void mesh_construction_data::set_max_instance_count(u32 count) {
		if (count == m_instance_count) return;
		if (!m_instanceFormat) {
			r2Error("Mesh does not have an instance format set, you can't allocate storage for instances for it");
			return;
		}

		if (count < m_last_instance_idx) {
			r2Error("Setting mesh max instance count to %d would destroy instance data, there are currently %d instances of this mesh", count, m_last_instance_idx);
			return;
		}

		resize_memory((void**)&m_instances, m_last_instance_idx, count, m_instanceFormat->size());
		m_instance_count = count;
	}

	bool mesh_construction_data::append_vertex_data(void* vdata, size_t count) {
		if (count == 0) return false;
		if (!m_vertexFormat) {
			r2Error("Mesh does not have an vertex format set, you can't append a vertex to it");
			return false;
		}

		if (m_last_vertex_idx + count > m_vertex_count) {
			r2Error("Cannot append vertex data to mesh, the mesh's maximum vertex count (%d) would be exceeded. Resize with mesh.set_max_vertex_count()", m_vertex_count);
			return false;
		}

		assign_memory(m_vertices, vdata, m_last_vertex_idx, m_vertexFormat->size() * count);
		m_last_vertex_idx += count;
		return true;
	}

	bool mesh_construction_data::append_index_data(void* idata, size_t count) {
		if (count == 0) return false;
		if (m_last_index_idx + count > m_index_count) {
			r2Error("Cannot append index data to mesh, the mesh's maximum index count (%d) would be exceeded. Resize with mesh.set_max_index_count()", m_index_count);
			return false;
		}

		assign_memory(m_indices, idata, m_last_index_idx, m_indexType * count);
		m_last_index_idx += count;
		return true;
	}

	bool mesh_construction_data::append_instance_data(void* idata, size_t count) {
		if (count == 0) return false;
		if (!m_instanceFormat) {
			r2Error("Mesh does not have an instance format set, you can't append an instance to it");
			return false;
		}

		if (m_last_instance_idx + count > m_instance_count) {
			r2Error("Cannot append instance data to mesh, the mesh's maximum instance count (%d) would be exceeded. Resize with mesh.set_max_instance_count()", m_instance_count);
			return false;
		}

		assign_memory(m_instances, idata, m_last_instance_idx, m_instanceFormat->size() * count);
		m_last_instance_idx += count;
		return true;
	}

	void mesh_construction_data::fill() {
		m_last_vertex_idx = m_vertex_count;
		if (m_vertices) memset(m_vertices, 0, m_vertexFormat->size() * size_t(m_vertex_count));
		
		m_last_index_idx = m_index_count;
		if (m_indices) memset(m_indices, 0, size_t(m_indexType) * size_t(m_index_count));

		m_last_instance_idx = m_instance_count;
		if (m_instances) memset(m_instances, 0, m_instanceFormat->size() * size_t(m_instance_count));
	}

	bool mesh_construction_data::update_instance(u32 idx, void* idata) {
		if (!m_instanceFormat) {
			r2Error("Mesh does not have an instance format set, you can't update an instance on it");
			return false;
		}

		if (idx > m_last_instance_idx) {
			r2Error("Instance index %d is out of bounds (mesh has %d instances)", idx, m_last_index_idx);
			return false;
		}

		assign_memory(m_instances, idata, idx, m_instanceFormat->size());
		return true;
	}
};
