#include <r2/engine.h>
#include <r2/managers/renderman.h>
#include <r2/utilities/texture.h>

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
		if (m_node) m_node->release(m_id);
		m_node = nullptr;
		m_id = -1;
	}

	render_node_instance::operator bool() const {
		return m_node && m_node->instance_valid(m_id);
	}

	void render_node_instance::update_instance_transform(const mat4f& transform) {
		if (!m_node) {
			r2Error("Invalid render_node_instance cannot be updated");
			return;
		}

		m_node->update_instance_transform(m_id, transform);
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

		m_node->update_vertices_raw(data, count);
	}

	void render_node_instance::update_indices_raw(const void* data, size_t count) {
		if (!m_node) {
			r2Error("Invalid render_node_instance cannot be updated");
			return;
		}

		m_node->update_indices_raw(data, count);
	}



    // render node
    render_node::render_node(scene* s, const vtx_bo_segment& vertData, idx_bo_segment* indexData, ins_bo_segment* instanceData) {
		m_scene = s;
        m_vertexData = vertData;
		m_material = nullptr;
		m_nextInstanceIdx = 0;
		m_uniforms = nullptr;

		m_vertexCount = vertData.size();
		m_indexCount = indexData ? indexData->size() : 0;

        if(indexData) m_indexData = idx_bo_segment(*indexData);
        if(instanceData) m_instanceData = ins_bo_segment(*instanceData);

		destroy_when_unused = false;
		has_transparency = false;
		primitives = pt_triangles;
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

	uniform_block* render_node::uniforms() const {
		return m_uniforms;
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

		if (m_nextInstanceIdx == 0 && destroy_when_unused) {
			m_scene->remove_node(this);
		}
	}

	void render_node::update_instance_transform(instanceId id, const mat4f& transform) {
		auto i = m_instanceIndices.find(id);
		if (i == m_instanceIndices.end()) {
			r2Error("render_node_instance %llu is invalid and cannot be updated", id);
			return;
		}

		if (!m_instanceData.buffer->format()->hasModelMatrix()) {
			r2Error("render_node_instance %llu's instance format '%s' has no model matrix attribute specified, render_node::update_instance_transform ignored", id, m_instanceData.buffer->format()->to_string().c_str());
			return;
		}
		
		size_t transformOffset = m_instanceData.buffer->format()->modelMatrixOffset();
		size_t idx = i->second;
		size_t instanceSize = m_instanceData.buffer->format()->size();
		size_t memBegin = (idx * instanceSize) + transformOffset;
		size_t memEnd = memBegin + sizeof(mat4f);
		ins_bo_segment seg = m_instanceData.sub(idx, idx + 1, memBegin, memEnd);
		m_instanceData.buffer->update(seg, &transform[0].x);
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

	void* render_node::instance_data(instanceId id) {
		auto i = m_instanceIndices.find(id);
		if (i == m_instanceIndices.end()) {
			r2Error("render_node_instance %llu is invalid has no instance data", id);
			return nullptr;
		}

		size_t idx = i->second;
		size_t instanceSize = m_instanceData.buffer->format()->size();
		return ((u8*)m_instanceData.buffer->data()) + m_instanceData.memBegin + (idx * instanceSize);
	}
	
	void render_node::update_vertices_raw(const void* data, size_t count) {
		if (count > m_vertexData.size()) {
			r2Error("More vertices (%llu) supplied to render_node_instance than its node can contain. Increase the max vertex count when creating the mesh", count);
			return;
		}

		size_t size = m_vertexData.buffer->format()->size();
		m_vertexData.buffer->update(m_vertexData.sub(0, count, 0, count * size), data);
		m_vertexCount = count;
	}

	void* render_node::vertex_data() {
		return ((u8*)m_vertexData.buffer->data()) + m_vertexData.memBegin;
	}

	void render_node::update_indices_raw(const void* data, size_t count) {
		if (count > m_indexData.size()) {
			r2Error("More indices (%llu) supplied to render_node_instance than its node can contain. Increase the max index count when creating the mesh", count);
			return;
		}

		size_t size = m_indexData.buffer->type();
		m_indexData.buffer->update(m_indexData.sub(0, count, 0, count * size), data);
		m_indexCount = count;
	}

	void* render_node::index_data() {
		return ((u8*)m_indexData.buffer->data()) + m_indexData.memBegin;
	}
	
	bool render_node::instance_valid(instanceId id) const {
		return m_instanceIndices.find(id) != m_instanceIndices.end();
	}

	void render_node::set_vertex_count(size_t count) {
		if (count > max_vertex_count()) {
			r2Error("Can't set node vertex count to a value higher than the node's vertex capacity.");
			return;
		}

		m_vertexCount = count;
	}

	void render_node::set_index_count(size_t count) {
		if (count > max_index_count()) {
			r2Error("Can't set node index count to a value higher than the node's index capacity.");
			return;
		}

		m_indexCount = count;
	}

	void render_node::add_uniform_block(uniform_block* uniforms) {
		for (uniform_block* block : m_userUniforms) {
			if (block == uniforms) {
				return;
			}
		}

		m_userUniforms.push_back(uniforms);
	}

	void render_node::remove_uniform_block(uniform_block* uniforms) {
		for (auto i = m_userUniforms.begin();i != m_userUniforms.end();i++) {
			if (*i == uniforms) {
				m_userUniforms.erase(i);
				return;
			}
		}

		r2Error("Uniform block does not exist in render node. Ignoring");
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
		uniform_block* b = nullptr;
		if (m_format) b = s->allocate_uniform_block(m_shaderBlockName, m_format);
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
		if (!m_uniforms) {
			info.GetReturnValue().Set(v8::Null(info.GetIsolate()));
			return;
		}
		info.GetReturnValue().Set(v8pp::convert<uniform_block>::to_v8(info.GetIsolate(), *m_uniforms));
	}

	void node_material_instance::set_texture(const mstring& uniformName, texture_buffer* tex) {
		u32 loc = m_material->shader()->get_uniform_location(uniformName);
		if (loc == u32(-1)) {
			r2Error("Shader '%s' has no sampler uniform named '%s'", m_material->shader()->name().c_str(), uniformName.c_str());
			return;
		}

		for(auto& existing : m_textures) {
			if (existing.location == loc) {
				r2free(existing.textures);
				existing.count = 1;
				existing.currentFrame = 0;
				existing.elapsed = 0.0f;
				existing.duration = 0.0f;
				existing.loop = false;
				texture_buffer** arr = new texture_buffer*[1];
				arr[0] = tex;
				existing.textures = arr;
				return;
			}
		}

		texture_buffer** arr = new texture_buffer*[1];
		arr[0] = tex;

		texture_uniform u = { loc, 1, 0, 0.0f, 0.0f, false, arr };
		m_textures.push_back(u);
	}
	void node_material_instance::set_texture(const mstring& uniformName, const mvector<texture_buffer*>& textures, f32 duration, bool loop) {
		if (textures.size() == 1) {
			set_texture(uniformName, textures[0]);
			return;
		}

		u32 loc = m_material->shader()->get_uniform_location(uniformName);
		if (loc == u32(-1)) {
			r2Error("Shader '%s' has no sampler uniform named '%s'", m_material->shader()->name().c_str(), uniformName.c_str());
			return;
		}

		for(auto& existing : m_textures) {
			if (existing.location == loc) {
				existing.count = textures.size();
				existing.currentFrame = 0;
				existing.elapsed = 0.0f;
				existing.duration = duration;
				existing.loop = loop;
				r2free(existing.textures);
				existing.textures = new texture_buffer*[textures.size() + 1];
				for (u32 i = 0;i < textures.size();i++) {
					existing.textures[i] = textures[i];
				}
				return;
			}
		}
		texture_buffer** arr = new texture_buffer*[textures.size()];
		for (u32 i = 0;i < textures.size();i++) {
			arr[i] = textures[i];
		}
		texture_uniform u = { loc, textures.size(), 0, 0.0f, duration, loop, arr };
		m_textures.push_back(u);
	}


    // scene
    scene::scene(scene_man* m,const mstring& name) {
		camera = nullptr;
		m_renderTarget = nullptr;

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
		vertex_buffer* vbo = vpool->find_buffer<vertex_buffer>(mesh->vertexFormat()->size() * mesh->vertex_count(), mesh->vertexFormat(), max((u32)DEFAULT_MAX_VERTICES, mesh->vertex_count()));
		vboData = vbo->append(mesh->vertex_data(), mesh->vertex_count());

		// indices
		if(mesh->index_count() > 0) {
			buffer_pool* pool = &m_idx_buffers[mesh->indexType()];
			index_buffer* ibo = pool->find_buffer<index_buffer>((size_t)mesh->indexType() * mesh->index_count(), mesh->indexType(), max((u32)DEFAULT_MAX_INDICES, mesh->index_count()));
			iboData = ibo->append(mesh->index_data(), mesh->index_count());
			iboDataPtr = &iboData;
		}

		// instances
		if(mesh->instance_count() > 0) {
			buffer_pool* pool = &m_ins_buffers[mesh->instanceFormat()->hash_name()];
			instance_buffer* ibo = pool->find_buffer<instance_buffer>(mesh->instanceFormat()->size() * mesh->instance_count(), mesh->instanceFormat(), max((u32)DEFAULT_MAX_INSTANCES, mesh->instance_count()));
			instanceData = ibo->append(mesh->instance_data(), mesh->instance_count());
			instanceDataPtr = &instanceData;
		}

		render_node* node = new render_node(this, vboData, iboDataPtr, instanceDataPtr);
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

	texture_buffer* scene::create_texture() {
		texture_buffer* tex = new texture_buffer();
		m_textures.push_back(tex);
		return tex;
	}

	render_buffer* scene::create_render_target() {
		render_buffer* buf = new render_buffer();
		m_targets.push_back(buf);
		return buf;
	}

	void scene::destroy(render_buffer* rbo) {
		for (auto i = m_targets.begin();i != m_targets.end();i++) {
			if ((*i) == rbo) {
				m_targets.erase(i);
				return;
			}
		}

		r2engine::renderer()->driver()->free_render_target(rbo);

		r2Error("scene::destroy was called with a render buffer that doesn't exist within this scene");
	}

	void scene::destroy(shader_program* program) {
		for (auto i = m_shaders.begin();i != m_shaders.end();i++) {
			if ((*i) == program) {
				m_shaders.erase(i);
				return;
			}
		}

		r2engine::assets()->destroy(program);

		r2Error("scene::destroy was called with a shader program that doesn't exist within this scene");
	}

	void scene::destroy(texture_buffer* texture) {
		for (auto i = m_textures.begin();i != m_textures.end();i++) {
			if ((*i) == texture) {
				m_textures.erase(i);
				return;
			}
		}

		r2engine::renderer()->driver()->free_texture(texture);

		r2Error("scene::destroy was called with a texture buffer that doesn't exist within this scene");
	}

	void scene::set_render_target(render_buffer* target) {
		m_renderTarget = target;
	}

	render_buffer* scene::render_target() {
		return m_renderTarget;
	}

	bool scene::remove_node(render_node* node) {
		render_driver* driver = r2engine::renderer()->driver();
		if (!driver) {
			r2Error("scene::remove_node was called, but there is no driver set.");
			return false;
		}

		if (node->instance_count() > 0) {
			r2Error("Can't remove node that has instances from the scene. Destroy the instances first.");
			return false;
		}

		for (auto it = m_nodes.begin();it != m_nodes.end();it++) {
			if ((*it) == node) {
				driver->free_vao(node);
				delete node->m_uniforms;
				delete node;
				m_nodes.erase(it);
				return true;
			}
		}

		r2Error("Node not found in scene...");
		return false;
	}

	void scene::generate_vaos() {
		render_driver* driver = r2engine::renderer()->driver();
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
		for(auto tex : m_textures) driver->sync_texture(tex);
		for(auto trg : m_targets) driver->sync_render_target(trg);
	}

	void scene::render(f32 dt) {
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
			mat4f invView = glm::inverse(view);

			m_sceneUniforms->uniform_mat4f("view", view);
			m_sceneUniforms->uniform_mat4f("invView", invView);
			m_sceneUniforms->uniform_mat4f("projection", proj);
			m_sceneUniforms->uniform_mat4f("view_proj", proj * view);

			vec3f pos = invView[3];
			f32 c = proj[2][2];
			f32 d = proj[2][3];
			f32 near = d / (c - 1.0f);
			f32 far = d / (c + 1.0f);
			m_sceneUniforms->uniform_vec3f("camera_pos", invView[3]);
			m_sceneUniforms->uniform_vec3f("camera_left", invView[0]);
			m_sceneUniforms->uniform_vec3f("camera_up", invView[1]);
			m_sceneUniforms->uniform_vec3f("camera_forward", invView[2]);
			m_sceneUniforms->uniform_float("camera_near", near);
			m_sceneUniforms->uniform_float("camera_far", far);
			m_sceneUniforms->uniform_float("camera_fov", atanf(1.0f / proj[1][1]) * 2.0f);
		}

		generate_vaos();
		sync_buffers();

		mvector<render_node*> transparent;
		transparent.reserve(m_nodes.size());

		driver->bind_render_target(m_renderTarget);
		driver->clear_framebuffer(vec4f(0.0f, 0.0f, 0.0f, 0.0f), m_renderTarget ? m_renderTarget->depth_mode() != rbdm_no_depth : true);
		
		for (auto node : m_nodes) {
			node_material_instance* mat = node->material_instance();
			if (mat) {
				u8 tc = mat->texture_count();
				for (u8 t = 0;t < tc;t++) {
					auto* tex = mat->texture(t);
					if (tex->count > 1) {
						tex->elapsed += dt;
						if (tex->elapsed > tex->duration) tex->elapsed = tex->loop ? 0.0f : tex->duration;
						tex->currentFrame = u32(floor((tex->elapsed / tex->duration) * f32(tex->count - 1)));
					}
				}
			}

			if (node->vertex_count() == 0 || (node->indices().is_valid() && node->index_count() == 0)) continue;

			if (node->has_transparency) transparent.push_back(node);
			else driver->render_node(node, m_sceneUniforms);
		}

		if (transparent.size() > 0) {
			for (auto node : transparent) {
				driver->render_node(node, m_sceneUniforms);
			}
		}

		driver->bind_render_target(nullptr);
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


		for(auto tex : m_textures) {
			driver->free_texture(tex);
			delete tex;
		}
		m_textures.clear();
		
		for(auto trg : m_targets) {
			driver->free_render_target(trg);
			delete trg;
		}
		m_targets.clear();
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
