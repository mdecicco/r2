#include <r2/engine.h>
#include <r2/managers/renderman.h>

namespace r2 {
    // render node
    render_node::render_node(const vtx_bo_segment& vertData, mesh_construction_data* cdata, idx_bo_segment* indexData, ins_bo_segment* instanceData) {
        m_vertexData = vertData;
		m_constructionData = cdata;
		m_indexData = idx_bo_segment(); memset(&m_indexData, 0, sizeof(idx_bo_segment));
		m_instanceData = ins_bo_segment(); memset(&m_instanceData, 0, sizeof(ins_bo_segment));
		material = nullptr;

        if(indexData) m_indexData = idx_bo_segment(*indexData);
        if(instanceData) m_instanceData = ins_bo_segment(*instanceData);
    }

    render_node::~render_node() {
		// TODO: release gpu buffer segments
		delete m_constructionData;
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



	// node material
	node_material::node_material(const string& shaderBlockName, const uniform_format& fmt) {
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

	const uniform_format& node_material::format() const {
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



    // scene
    scene::scene(scene_man* m,const string& name) {
        m_mgr = m;
        m_name = name;
        r2Log("Scene created (%s)",m_name.c_str());
    }

    scene::~scene() {
        for(auto i = m_nodes.begin();i != m_nodes.end();i++) {
            delete *i;
        }
        m_nodes.clear();
        r2Log("Scene destroyed (%s)",m_name.c_str());
    }

    scene_man* scene::manager() const {
        return m_mgr;
    }

    string scene::name() const {
        return m_name;
    }

    bool scene::operator==(const scene& rhs) const {
        return m_name == rhs.name();
    }

	render_node* scene::add_mesh(mesh_construction_data* mesh) {
		if(!check_mesh(mesh->vertex_count())) return nullptr;

		vtx_bo_segment vboData;

		// optional
		idx_bo_segment iboData;
		idx_bo_segment* iboDataPtr = nullptr;
		ins_bo_segment instanceData;
		ins_bo_segment* instanceDataPtr = nullptr;

		// vertices
		buffer_pool* vpool = &m_vtx_buffers[mesh->vertexFormat().hash_name()];
		vertex_buffer* vbo = vpool->find_buffer<vertex_buffer>(mesh->vertexFormat().size() * mesh->vertex_count(), mesh->vertexFormat(), DEFAULT_MAX_VERTICES);
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
			buffer_pool* pool = &m_ins_buffers[mesh->instanceFormat().hash_name()];
			instance_buffer* ibo = pool->find_buffer<instance_buffer>(mesh->instanceFormat().size() * mesh->instance_count(), mesh->instanceFormat(), DEFAULT_MAX_INSTANCES);
			instanceData = ibo->append(mesh->instance_data(), mesh->instance_count());
			instanceDataPtr = &instanceData;
		}

		render_node* node = new render_node(vboData, mesh, iboDataPtr, instanceDataPtr);
		m_nodes.push_back(node);

		return node;
	}

	uniform_block* scene::allocate_uniform_block(const string& name, const uniform_format& fmt) {
		buffer_pool* pool = &m_ufm_buffers[fmt.hash_name()];
		uniform_buffer* ubo = pool->find_buffer<uniform_buffer>(fmt.size(), fmt, DEFAULT_MAX_UNIFORM_BLOCKS);
		u8* data = new u8[fmt.size()];
		memset(data, 0, fmt.size());
		ufm_bo_segment seg = ubo->append(data, 1);
		delete[] data;

		return new uniform_block(name, seg);
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

		for(auto node : m_nodes) {
			driver->render_node(node);
		}
	}

    bool scene::check_mesh(size_t vc) const {
        if(vc == 0) {
            r2Error("Call to scene::add_mesh failed. Mesh has no vertices");
            return false;
        }
        return true;
    }



    // manager
    scene_man::scene_man(r2engine* e) {
        m_eng = e;
        r2Log("Scene manager initialized");
    }

    scene_man::~scene_man() {
        for(auto i = m_scenes.begin();i != m_scenes.end();i++) {
            r2Warn("Scene %s not destroyed before engine shutdown. Destroying now to prevent memory leak",(*i)->name().c_str());
            delete *i;
        }
        m_scenes.clear();
        r2Log("Scene manager destroyed");
    }

    r2engine* scene_man::engine() const {
        return m_eng;
    }

    scene* scene_man::create(const string &name) {
        for(auto i = m_scenes.begin();i != m_scenes.end();i++) {
            if((*i)->name() == name) {
                r2Warn("Call to scene_man::create failed: A scene with the name \'%s\' already exists",name.c_str());
                return nullptr;
            }
        }
        scene* s = new scene(this,name);
        m_scenes.push_back(s);
        return s;
    }

	scene* scene_man::get(const string& name) {
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
                delete s;
                m_scenes.erase(i);
                return;
            }
        }
        r2Error("Call to scene_man::destroy failed: Provided scene not found");
    }
}
