#include <r2/engine.h>
#include <r2/managers/renderman.h>

#include <r2/systems/camera_sys.h>
#include <r2/systems/transform_sys.h>
#include <r2/systems/cascade_functions.h>

namespace r2 {
	// render node instance
	instanceId render_node_instance::nextInstanceId = 1;
	render_node_instance::render_node_instance() : m_node(nullptr), m_id(-1) { }
	render_node_instance::render_node_instance(const render_node_instance& o) : m_node(o.m_node), m_id(o.m_id) { }
	render_node_instance::render_node_instance(render_node* node) : m_node(node), m_id(render_node_instance::nextInstanceId++) { }
	render_node_instance::~render_node_instance() { }


	void render_node_instance::release() {
		if (m_node) m_node->release(*this);
	}

	render_node_instance::operator bool() const {
		return m_node && m_node->instance_valid(m_id);
	}

	void render_node_instance::update_instance_raw(const void* data) {
		if (!m_node) {
			r2Error("Invalid render_node_instance cannot be updated");
			return;
		}

		m_node->update_instance_raw(m_id, data);
	}

	void render_node_instance::update_vertices_raw(const void* data, size_t count) {
		if (!m_node) {
			r2Error("Invalid render_node_instance cannot be updated");
			return;
		}

		m_node->update_vertices_raw(m_id, data, count);
	}

	void render_node_instance::update_indices_raw(const void* data, size_t count) {
		if (!m_node) {
			r2Error("Invalid render_node_instance cannot be updated");
			return;
		}

		m_node->update_indices_raw(m_id, data, count);
	}



    // render node
    render_node::render_node(const vtx_bo_segment& vertData, mesh_construction_data* cdata, idx_bo_segment* indexData, ins_bo_segment* instanceData) {
        m_vertexData = vertData;
		m_constructionData = cdata;
		m_material = nullptr;
		m_nextInstanceIdx = 0;
		m_uniforms = nullptr;

		m_vertexCount = cdata->vertex_count();
		m_indexCount = cdata->index_count();

        if(indexData) m_indexData = idx_bo_segment(*indexData);
        if(instanceData) m_instanceData = ins_bo_segment(*instanceData);
    }

    render_node::~render_node() {
		// TODO: release gpu buffer segments

    }

	const vtx_bo_segment& render_node::vertices() const {
		return m_vertexData;
	}

	const idx_bo_segment& render_node::indices() const {
		return m_indexData;
	}

	const ins_bo_segment& render_node::instances() const {
		return m_instanceData;
	}

	void render_node::set_material_instance(node_material_instance* material) {
		m_material = material;
	}

	node_material_instance* render_node::material_instance() const {
		return m_material;
	}

	render_node_instance render_node::instantiate() {
		size_t remainingSlots = m_instanceData.size() - m_nextInstanceIdx;
		if (remainingSlots == 0) {
			r2Error("No more instances can fit in the instance buffer (buf: %llu)", m_instanceData.buffer->id());
			return render_node_instance();
		}

		render_node_instance out(this);
		m_instanceIndices[out.id()] = m_nextInstanceIdx++;
		return out;
	}

	void render_node::release(instanceId id) {
		auto i = m_instanceIndices.find(id);
		if (i == m_instanceIndices.end()) {
			r2Error("render_node_instance %llu is invalid and cannot be released", id);
			return;
		}

		// don't allow gaps in instance indices
		// they should always remain sequential
		size_t idx = i->second;
		for(auto instance : m_instanceIndices) {
			if (instance.second > idx) m_instanceIndices[instance.first]--;
		}

		m_nextInstanceIdx--;
		m_instanceIndices.erase(i);
	}

	void render_node::update_instance_raw(instanceId id, const void* data) {
		auto i = m_instanceIndices.find(id);
		if (i == m_instanceIndices.end()) {
			r2Error("render_node_instance %llu is invalid and cannot be updated", id);
			return;
		}

		size_t idx = i->second;
		size_t instanceSize = m_instanceData.buffer->format()->size();
		size_t memBegin = idx * instanceSize;
		size_t memEnd = memBegin + instanceSize;
		ins_bo_segment seg = m_instanceData.sub(idx, idx + 1, memBegin, memEnd);
		m_instanceData.buffer->update(seg, data);
	}
	
	void render_node::update_vertices_raw(instanceId id, const void* data, size_t count) {
		auto i = m_instanceIndices.find(id);
		if (i == m_instanceIndices.end()) {
			r2Error("render_node_instance %llu is invalid and cannot be updated", id);
			return;
		}

		if (count > m_vertexData.size()) {
			r2Error("More vertices (%llu) supplied to render_node_instance than its node can contain. Increase the max vertex count when creating the mesh", count);
			return;
		}

		size_t size = m_vertexData.buffer->format()->size();
		m_vertexData.buffer->update(m_vertexData.sub(0, count, 0, count * size), data);
		m_vertexCount = count;
	}

	void render_node::update_indices_raw(instanceId id, const void* data, size_t count) {
		auto i = m_instanceIndices.find(id);
		if (i == m_instanceIndices.end()) {
			r2Error("render_node_instance %llu is invalid and cannot be updated", id);
			return;
		}

		if (count > m_indexData.size()) {
			r2Error("More indices (%llu) supplied to render_node_instance than its node can contain. Increase the max index count when creating the mesh", count);
			return;
		}

		size_t size = m_indexData.buffer->type();
		m_indexData.buffer->update(m_indexData.sub(0, count, 0, count * size), data);
		m_indexCount = count;
	}
	
	bool render_node::instance_valid(instanceId id) const {
		return m_instanceIndices.find(id) != m_instanceIndices.end();
	}



	// node material
	node_material::node_material(const mstring& shaderBlockName, uniform_format* fmt) {
		m_shaderBlockName = shaderBlockName;
		m_format = fmt;
	}

	node_material::~node_material() {
	}

	void node_material::set_shader(shader_program* shader) {
		m_shader = shader;
	}

	shader_program* node_material::shader() const {
		return m_shader;
	}

	uniform_format* node_material::format() const {
		return m_format;
	}

	node_material_instance* node_material::instantiate(scene* s) {
		uniform_block* b = s->allocate_uniform_block(m_shaderBlockName, m_format);
		return new node_material_instance(this, b);
	}



	// node material instance
	node_material_instance::node_material_instance(node_material* material, uniform_block* uniforms) {
		m_material = material;
		m_uniforms = uniforms;
	}
	
	node_material_instance::~node_material_instance() {
		m_material = nullptr;
		delete m_uniforms;
	}
	
	node_material* node_material_instance::material() const {
		return m_material;
	}

	uniform_block* node_material_instance::uniforms() const {
		return m_uniforms;
	}
	void node_material_instance::uniforms_v8(v8::Local<v8::String> name, v8::PropertyCallbackInfo<v8::Value> const& info) {
		info.GetReturnValue().Set(v8pp::convert<uniform_block>::to_v8(info.GetIsolate(), *m_uniforms));
	}



    // scene
    scene::scene(scene_man* m,const mstring& name) {
		camera = nullptr;

        m_mgr = m;
        m_name = name;
		m_sceneUniforms = allocate_uniform_block("u_scene", static_uniform_formats::scene());
        r2Log("Scene created (%s)", m_name.c_str());
    }

    scene::~scene() {
        r2Log("Scene destroyed (%s)", m_name.c_str());
    }

    scene_man* scene::manager() const {
        return m_mgr;
    }

    const mstring& scene::name() const {
        return m_name;
    }

    bool scene::operator==(const scene& rhs) const {
        return m_name == rhs.name();
    }

	render_node* scene::add_mesh(mesh_construction_data* mesh) {
		if (mesh->m_wasSentToGpu) {
			r2Error("Mesh was already made renderable.");
			return nullptr;
		}
		if (!check_mesh(mesh->vertex_count())) return nullptr;

		vtx_bo_segment vboData;

		// optional
		idx_bo_segment iboData;
		idx_bo_segment* iboDataPtr = nullptr;
		ins_bo_segment instanceData;
		ins_bo_segment* instanceDataPtr = nullptr;

		// vertices
		buffer_pool* vpool = &m_vtx_buffers[mesh->vertexFormat()->hash_name()];
		vertex_buffer* vbo = vpool->find_buffer<vertex_buffer>(mesh->vertexFormat()->size() * mesh->vertex_count(), mesh->vertexFormat(), DEFAULT_MAX_VERTICES);
		vboData = vbo->append(mesh->vertex_data(), mesh->vertex_count());

		// indices
		if(mesh->index_count() > 0) {
			buffer_pool* pool = &m_idx_buffers[mesh->indexType()];
			index_buffer* ibo = pool->find_buffer<index_buffer>((size_t)mesh->indexType() * mesh->index_count(), mesh->indexType(), DEFAULT_MAX_INDICES);
			iboData = ibo->append(mesh->index_data(), mesh->index_count());
			iboDataPtr = &iboData;
		}

		// instances
		if(mesh->instance_count() > 0) {
			buffer_pool* pool = &m_ins_buffers[mesh->instanceFormat()->hash_name()];
			instance_buffer* ibo = pool->find_buffer<instance_buffer>(mesh->instanceFormat()->size() * mesh->instance_count(), mesh->instanceFormat(), DEFAULT_MAX_INSTANCES);
			instanceData = ibo->append(mesh->instance_data(), mesh->instance_count());
			instanceDataPtr = &instanceData;
		}

		render_node* node = new render_node(vboData, mesh, iboDataPtr, instanceDataPtr);
		m_nodes.push_back(node);

		mesh->m_wasSentToGpu = true;
		delete[] mesh->m_vertices;
		mesh->m_vertices = nullptr;
		if (mesh->m_indices) { delete[] mesh->m_indices; mesh->m_indices = nullptr; }
		if (mesh->m_instances) { delete[] mesh->m_instances; mesh->m_instances = nullptr; }

		node->m_uniforms = allocate_uniform_block("u_model", static_uniform_formats::node());

		return node;
	}

	uniform_block* scene::allocate_uniform_block(const mstring& name, uniform_format* fmt) {
		buffer_pool* pool = &m_ufm_buffers[fmt->hash_name()];
		uniform_buffer* ubo = pool->find_buffer<uniform_buffer>(fmt->size(), fmt, DEFAULT_MAX_UNIFORM_BLOCKS);
		u8* data = new u8[fmt->size()];
		memset(data, 0, fmt->size());
		ufm_bo_segment seg = ubo->append(data);
		delete[] data;

		return new uniform_block(name, seg);
	}

	shader_program* scene::load_shader(const mstring& file, const mstring& assetName) {
		render_driver* driver = r2engine::get()->renderer()->driver();
		if (!driver) {
			r2Error("scene::load_shader was called, but there is no driver set.");
			return nullptr;
		}
		
		shader_program* shader = driver->load_shader(file, assetName);
		if (shader) {
			m_shaders.push_back(shader);
		}

		return shader;
	}

	void scene::generate_vaos() {
		render_driver* driver = r2engine::get()->renderer()->driver();
		if (!driver) {
			r2Error("scene::generate_vaos was called, but there is no driver set.");
			return;
		}

		for(render_node* node : m_nodes) driver->generate_vao(node);
	}

	void scene::sync_buffers() {
		render_driver* driver = r2engine::get()->renderer()->driver();
		if (!driver) {
			r2Error("scene::sync_buffers was called, but there is no driver set.");
			return;
		}

		for(auto buf : m_vtx_buffers) buf.second.sync_buffers(driver);
		for(auto buf : m_idx_buffers) buf.second.sync_buffers(driver);
		for(auto buf : m_ins_buffers) buf.second.sync_buffers(driver);
		for(auto buf : m_ufm_buffers) buf.second.sync_buffers(driver);
	}

	void scene::render() {
		render_driver* driver = r2engine::get()->renderer()->driver();
		if (!driver) {
			r2Error("scene::sync_buffers was called, but there is no driver set.");
			return;
		}

		// TODO: Optimize

		if (camera && camera->camera) {
			mat4f proj = camera->camera->projection;
			mat4f view = mat4f(1.0f);
			if (camera->transform) {
				view = camera->transform->cascaded_property(&transform_component::transform, &cascade_mat4f);
			}

			m_sceneUniforms->uniform_mat4f("transform", view);
			m_sceneUniforms->uniform_mat4f("projection", proj);
			m_sceneUniforms->uniform_mat4f("view_proj", proj * view);
		}

		generate_vaos();
		sync_buffers();

		for(auto node : m_nodes) {
			driver->render_node(node, m_sceneUniforms);
		}
	}

	void scene::release_resources() {
		render_driver* driver = r2engine::get()->renderer()->driver();
		if (!driver) {
			r2Error("scene::sync_buffers was called, but there is no driver set.");
			return;
		}

		for(auto shader : m_shaders) {
			r2engine::get()->assets()->destroy(shader);
		}
		m_shaders.clear();

		for(auto node : m_nodes) {
			driver->free_vao(node);
			delete node->m_uniforms;
			delete node;
		}

		m_nodes.clear();

		delete m_sceneUniforms;
		m_sceneUniforms = nullptr;

		for(auto buf : m_vtx_buffers) buf.second.free_buffers(driver);
		m_vtx_buffers.clear();

		for(auto buf : m_idx_buffers) buf.second.free_buffers(driver);
		m_idx_buffers.clear();

		for(auto buf : m_ins_buffers) buf.second.free_buffers(driver);
		m_ins_buffers.clear();

		for(auto buf : m_ufm_buffers) buf.second.free_buffers(driver);
		m_ufm_buffers.clear();
	}

    bool scene::check_mesh(size_t vc) const {
        if(vc == 0) {
            r2Error("Call to scene::add_mesh failed. Mesh has no vertices");
            return false;
        }
        return true;
    }



    // manager
    scene_man::scene_man() {
        r2Log("Scene manager initialized");
    }

    scene_man::~scene_man() {
		memory_man::push_current(memory_man::global());
        for(auto i = m_scenes.begin();i != m_scenes.end();i++) {
            r2Warn("Scene %s not destroyed before engine shutdown. Destroying now to prevent memory leak", (*i)->name().c_str());
			(*i)->release_resources();
            delete *i;
        }
        m_scenes.clear();
		memory_man::pop_current();
        r2Log("Scene manager destroyed");
    }

    scene* scene_man::create(const mstring &name) {
		if (!r2engine::get()->renderer()->driver()) {
			r2Error("A scene can't be created until a render driver is specified");
			return nullptr;
		}
        for(auto i = m_scenes.begin();i != m_scenes.end();i++) {
            if((*i)->name() == name) {
                r2Warn("Call to scene_man::create failed: A scene with the name \'%s\' already exists",name.c_str());
                return nullptr;
            }
        }
        scene* s = new scene(this, name);
		memory_man::push_current(memory_man::global());
        m_scenes.push_back(s);
		memory_man::pop_current();
        return s;
    }

	scene* scene_man::get(const mstring& name) {
		for(auto i = m_scenes.begin();i != m_scenes.end();i++) {
			if((*i)->name() == name) {
				return *i;
			}
		}
		return nullptr;
	}

    void scene_man::destroy(scene* s) {
        if(!s) {
            r2Error("Call to scene_man::destroy failed: null pointer provided");
            return;
        }

        for(auto i = m_scenes.begin();i != m_scenes.end();i++) {
            if(*(*i) == *s) {
				s->release_resources();
                delete s;
				memory_man::push_current(memory_man::global());
                m_scenes.erase(i);
				memory_man::pop_current();
                return;
            }
        }
        r2Error("Call to scene_man::destroy failed: Provided scene not found");
    }
}
