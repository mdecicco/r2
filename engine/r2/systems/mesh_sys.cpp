#include <r2/engine.h>
#include <r2/systems/mesh_sys.h>
#include <r2/bindings/graphics_bindings.h>
#include <r2/bindings/math_converters.h>

namespace r2 {
	mesh_component::mesh_component() {
	}

	mesh_component::~mesh_component() {
	}

	void mesh_component::set_node(render_node* node) {
		if (m_instance) m_instance.release();
		m_instance = node->instantiate();
	}

	void mesh_component::release_node() {
		if (m_instance) m_instance.release();
	}

	render_node* mesh_component::get_node() {
		return m_instance.node();
	}

	void mesh_component::set_instance_data(v8Args args) {
		if (!m_instance.node()) {
			r2Error("Attempted to set mesh component's instance data before setting the node that this mesh is an instance of");
			return;
		}

		if (!m_instance) {
			r2Error("Attempted to set mesh component's instance data when the mesh doesn't have a valid reference to a node");
			return;
		}

		if (!args[0]->IsObject()) {
			r2Error("Attempted to set mesh component's instance data to something other than an object");
			return;
		}
		
		auto format = m_instance.node()->instances().buffer->format();
		size_t size = format->size();
		u8* instanceData = new u8[size];
		memset(instanceData, 0, size);
		if(!parse_instance(LocalObjectHandle::Cast(args[0]), format, instanceData)) {
			r2Error("Failed to parse instance data supplied to mesh component");
			delete [] instanceData;
			return;
		}

		m_instance.update_instance_raw(instanceData);
		delete [] instanceData;

		get_instance_data(args);
	}

	void mesh_component::get_instance_data(v8Args args) {
		if (!m_instance.node()) {
			r2Error("Attempted to get mesh component's instance data before setting the node that this mesh is an instance of");
			return;
		}

		if (!m_instance) {
			r2Error("Attempted to get mesh component's instance data when the mesh doesn't have a valid reference to a node");
			return;
		}

		auto format = m_instance.node()->instances().buffer->format();
		auto attribs = format->attributes();
		Local<Array> arr = Array::New(args.GetIsolate(), attribs.size());
		u8* idata = (u8*)m_instance.node()->instance_data(m_instance.id());
		for (u32 i = 0;i < attribs.size();i++) {
			instance_attribute_type t = attribs[i];
			switch (t) {
				case iat_int: {
					i32& val = *(i32*)(idata + format->offsetOf(i));
					arr->Set(i, v8pp::convert<i32>::to_v8(args.GetIsolate(), val));
					break;
				}
				case iat_float: {
					float& val = *(float*)(idata + format->offsetOf(i));
					arr->Set(i, v8pp::convert<float>::to_v8(args.GetIsolate(), val));
					break;
				}
				case iat_uint: {
					u32& val = *(u32*)(idata + format->offsetOf(i));
					arr->Set(i, v8pp::convert<u32>::to_v8(args.GetIsolate(), val));
					break;
				}
				case iat_vec2i: {
					vec2i& val = *(vec2i*)(idata + format->offsetOf(i));
					arr->Set(i, v8pp::convert<vec2i>::to_v8(args.GetIsolate(), val));
					break;
				}
				case iat_vec2f: {
					vec2f& val = *(vec2f*)(idata + format->offsetOf(i));
					arr->Set(i, v8pp::convert<vec2f>::to_v8(args.GetIsolate(), val));
					break;
				}
				case iat_vec2ui: {
					vec2ui& val = *(vec2ui*)(idata + format->offsetOf(i));
					arr->Set(i, v8pp::convert<vec2ui>::to_v8(args.GetIsolate(), val));
					break;
				}
				case iat_vec3i: {
					vec3i& val = *(vec3i*)(idata + format->offsetOf(i));
					arr->Set(i, v8pp::convert<vec3i>::to_v8(args.GetIsolate(), val));
					break;
				}
				case iat_vec3f: {
					vec3f& val = *(vec3f*)(idata + format->offsetOf(i));
					arr->Set(i, v8pp::convert<vec3f>::to_v8(args.GetIsolate(), val));
					break;
				}
				case iat_vec3ui: {
					vec3ui& val = *(vec3ui*)(idata + format->offsetOf(i));
					arr->Set(i, v8pp::convert<vec3ui>::to_v8(args.GetIsolate(), val));
					break;
				}
				case iat_vec4i: {
					vec4i& val = *(vec4i*)(idata + format->offsetOf(i));
					arr->Set(i, v8pp::convert<vec4i>::to_v8(args.GetIsolate(), val));
					break;
				}
				case iat_vec4f: {
					vec4f& val = *(vec4f*)(idata + format->offsetOf(i));
					arr->Set(i, v8pp::convert<vec4f>::to_v8(args.GetIsolate(), val));
					break;
				}
				case iat_vec4ui: {
					vec4ui& val = *(vec4ui*)(idata + format->offsetOf(i));
					arr->Set(i, v8pp::convert<vec4ui>::to_v8(args.GetIsolate(), val));
					break;
				}
				case iat_mat2i: {
					mat2i& val = *(mat2i*)(idata + format->offsetOf(i));
					arr->Set(i, v8pp::convert<mat2i>::to_v8(args.GetIsolate(), val));
					break;
				}
				case iat_mat2f: {
					mat2f& val = *(mat2f*)(idata + format->offsetOf(i));
					arr->Set(i, v8pp::convert<mat2f>::to_v8(args.GetIsolate(), val));
					break;
				}
				case iat_mat2ui: {
					mat2ui& val = *(mat2ui*)(idata + format->offsetOf(i));
					arr->Set(i, v8pp::convert<mat2ui>::to_v8(args.GetIsolate(), val));
					break;
				}
				case iat_mat3i: {
					mat3i& val = *(mat3i*)(idata + format->offsetOf(i));
					arr->Set(i, v8pp::convert<mat3i>::to_v8(args.GetIsolate(), val));
					break;
				}
				case iat_mat3f: {
					mat3f& val = *(mat3f*)(idata + format->offsetOf(i));
					arr->Set(i, v8pp::convert<mat3f>::to_v8(args.GetIsolate(), val));
					break;
				}
				case iat_mat3ui: {
					mat3ui& val = *(mat3ui*)(idata + format->offsetOf(i));
					arr->Set(i, v8pp::convert<mat3ui>::to_v8(args.GetIsolate(), val));
					break;
				}
				case iat_mat4i: {
					mat4i& val = *(mat4i*)(idata + format->offsetOf(i));
					arr->Set(i, v8pp::convert<mat4i>::to_v8(args.GetIsolate(), val));
					break;
				}
				case iat_mat4f: {
					mat4f& val = *(mat4f*)(idata + format->offsetOf(i));
					arr->Set(i, v8pp::convert<mat4f>::to_v8(args.GetIsolate(), val));
					break;
				}
				case iat_mat4ui: {
					mat4ui& val = *(mat4ui*)(idata + format->offsetOf(i));
					arr->Set(i, v8pp::convert<mat4ui>::to_v8(args.GetIsolate(), val));
					break;
				}
			};
		}

		args.GetReturnValue().Set(arr);
	}

	void mesh_component::get_max_vertex_count(v8Args args) {
		size_t count = m_instance.node()->vertices().size();
		args.GetReturnValue().Set(BigInt::New(args.GetIsolate(), count));
	}

	void mesh_component::get_vertex_count(v8Args args) {
		args.GetReturnValue().Set(BigInt::New(args.GetIsolate(), m_instance.node()->vertex_count()));
	}

	void mesh_component::set_vertex_data(v8Args args) {
		u8* data = nullptr;
		size_t count = 0;
		if (parse_vertices(args, m_instance.node()->vertices().buffer->format(), &data, &count)) {
			m_instance.update_vertices_raw(data, count);
			delete [] data;
		}
	}

	void mesh_component::get_vertex_data(v8Args args) {
		EscapableHandleScope scope(args.GetIsolate());
		auto node = m_instance.node();
		auto format = node->vertices().buffer->format();
		size_t vsize = format->size();
		u8* data = (u8*)node->vertices().buffer->data();
		size_t count = node->vertex_count();

		auto isolate = args.GetIsolate();
		Local<Array> arr = Array::New(isolate, count);
		#define attrCase(sv, t) case sv: { jsAttr = v8pp::convert<t>::to_v8(isolate, *(t*)(data + (i * vsize) + offset)); offset += sizeof(t); break; }

		bool bad = false;
		for(size_t i = 0;i < count;i++) {
			auto attrs = format->attributes();
			u8 attr_count = (u8)attrs.size();

			size_t offset = 0;
			Local<Array> varr = Array::New(isolate, attr_count);
			for(u8 a = 0;a < attr_count;a++) {
				vertex_attribute_type attr = attrs[a];
				Local<Value> jsAttr;
				try {
					switch(attr) {
						attrCase(vat_int, i32)
						attrCase(vat_float, f32)
						attrCase(vat_uint, u32)
						attrCase(vat_vec2i, vec2i)
						attrCase(vat_vec2f, vec2f)
						attrCase(vat_vec2ui, vec2ui)
						attrCase(vat_vec3i, vec3i)
						attrCase(vat_vec3f, vec3f)
						attrCase(vat_vec3ui, vec3ui)
						attrCase(vat_vec4i, vec4i)
						attrCase(vat_vec4f, vec4f)
						attrCase(vat_vec4ui, vec4ui)
						attrCase(vat_mat2i, mat2i)
						attrCase(vat_mat2f, mat2f)
						attrCase(vat_mat2ui, mat2ui)
						attrCase(vat_mat3i, mat3i)
						attrCase(vat_mat3f, mat3f)
						attrCase(vat_mat3ui, mat3ui)
						attrCase(vat_mat4i, mat4i)
						attrCase(vat_mat4f, mat4f)
						attrCase(vat_mat4ui, mat4ui)
						default: {
							r2Warn("Invalid vertex attribute type specified to vertex format.");
							bad = true;
						}
					}
				} catch (std::exception e) {
					bad = true;
				}
				if (bad) {
					r2Error("Failed to convert vertex attribute %d to JS object (format: %s)", a, format->to_string().c_str());
					break;
				}

				if(!varr->Set(a, jsAttr)) {
					r2Error("Failed to append vertex attribute %d to vertex %llu (format: %s)", a, i, format->to_string().c_str());
					bad = true;
				}
			}

			if (bad) break;

			if(!arr->Set(i, varr)) {
				r2Error("Failed to append vertex %llu to vertex array (format: %s)", i, format->to_string().c_str());
				bad = true;
			}
		}

		#undef attrCase

		if (bad) args.GetReturnValue().Set(Undefined(isolate));
		args.GetReturnValue().Set(scope.Escape(arr));
	}


	void mesh_component::get_max_index_count(v8Args args) {
		size_t count = m_instance.node()->indices().size();
		args.GetReturnValue().Set(BigInt::New(args.GetIsolate(), count));
	}

	void mesh_component::get_index_count(v8Args args) {
		args.GetReturnValue().Set(BigInt::New(args.GetIsolate(), m_instance.node()->index_count()));
	}

	void mesh_component::set_index_data(v8Args args) {
		u8* data = nullptr;
		size_t count = 0;
		if (parse_indices(args, m_instance.node()->indices().buffer->type(), &data, &count)) {
			m_instance.update_indices_raw(data, count);
			delete [] data;
		}
	}

	void mesh_component::get_index_data(v8Args args) {
		EscapableHandleScope scope(args.GetIsolate());
		auto node = m_instance.node();
		auto type = node->indices().buffer->type();
		u8* data = (u8*)node->indices().buffer->data();
		size_t count = node->index_count();

		auto isolate = args.GetIsolate();
		Local<Array> arr = Array::New(isolate, count);

		size_t offset = 0;
		bool bad = false;
		for(size_t i = 0;i < count;i++) {
			Local<Value> index;
			switch(type) {
				case it_unsigned_byte: {
					index = Number::New(isolate, data[i]);
					offset++;
					break;
				}
				case it_unsigned_short: {
					index = Number::New(isolate, *(u16*)(data + (i * type) + offset));
					offset += 2;
					break;
				}
				case it_unsigned_int: {
					index = Number::New(isolate, *(u32*)(data + (i * type) + offset));
					offset += 4;
					break;
				}
				default: {
					r2Warn("Invalid index attribute type specified.");
					bad = true;
				}
			}

			if (bad) break;

			if(!arr->Set(i, index)) {
				r2Error("Failed to append index %llu to index array", i);
				bad = true;
			}
		}

		if (bad) args.GetReturnValue().Set(Undefined(isolate));
		args.GetReturnValue().Set(scope.Escape(arr));
	}

	void mesh_component::set_instance_transform(const mat4f& transform) {
		m_instance.update_instance_transform(transform);
	}

	size_t mesh_component::get_max_vertex_count() {
		if (!m_instance) {
			r2Error("Attempted to get mesh component's max vertex count when the mesh doesn't have a valid reference to a node");
			return 0;
		}
		return m_instance.node()->vertices().size();
	}

	size_t mesh_component::get_vertex_count() {
		if (!m_instance) {
			r2Error("Attempted to get mesh component's vertex count when the mesh doesn't have a valid reference to a node");
			return 0;
		}
		return m_instance.node()->vertex_count();
	}

	size_t mesh_component::get_max_index_count() {
		if (!m_instance) {
			r2Error("Attempted to get mesh component's max index count when the mesh doesn't have a valid reference to a node");
			return 0;
		}
		return m_instance.node()->indices().size();
	}

	size_t mesh_component::get_index_count() {
		if (!m_instance) {
			r2Error("Attempted to get mesh component's index count when the mesh doesn't have a valid reference to a node");
			return 0;
		}
		return m_instance.node()->index_count();
	}



	mesh_sys* mesh_sys::instance = nullptr;
	mesh_sys::mesh_sys() {
	}

	mesh_sys::~mesh_sys() {
	}

	void mesh_sys::initialize_entity(scene_entity* entity) {
		if (!entity->is_scripted()) return;
		entity->bind(this, "add_mesh_component", [](entity_system* system, scene_entity* entity, v8Args args) {
			system->addComponentTo(entity);
		});
	}

	void mesh_sys::deinitialize_entity(scene_entity* entity) {
		if (!entity->is_scripted()) return;
		auto s = state();
		s.enable();
		if (!s->contains_entity(entity->id())) {
			entity->unbind("add_mesh_component");
		}
		s.disable();
	}

	scene_entity_component* mesh_sys::create_component(entityId id) {
		auto s = state();
		s.enable();
		auto out = s->create<mesh_component>(id);
		s.disable();
		return out;
	}

	void mesh_sys::bind(scene_entity_component* component, scene_entity* entity) {
		using c = mesh_component;
		if (entity->is_scripted()) {
			entity->unbind("add_mesh_component");
			entity->bind(this, "remove_mesh_component", [](entity_system* system, scene_entity* entity, v8Args args) {
				system->removeComponentFrom(entity);
			});

			bind_instance_data((mesh_component*)component, entity);
			bind_vertex_data((mesh_component*)component, entity);
			bind_index_data((mesh_component*)component, entity);
			bind_node((mesh_component*)component, entity);
		}

		entity->mesh = component_ref<mesh_component*>(this, component->id());
	}

	void mesh_sys::unbind(scene_entity* entity) {
		if (entity->is_scripted()) {
			entity->unbind("node");
			entity->unbind("instance");
			entity->unbind("max_vertex_count");
			entity->unbind("vertex_count");
			entity->unbind("get_vertices");
			entity->unbind("set_vertices");
			entity->unbind("max_index_count");
			entity->unbind("index_count");
			entity->unbind("get_indices");
			entity->unbind("set_indices");
			entity->bind(this, "add_mesh_component", [](entity_system* system, scene_entity* entity, v8Args args) {
				system->addComponentTo(entity);
			});
		}

		entity->mesh->release_node();
		entity->mesh.clear();
	}

	void mesh_sys::initialize() {
	}

	void mesh_sys::tick(f32 dt) {
	}

	void mesh_sys::handle(event* evt) {
	}

	void mesh_sys::bind_instance_data(mesh_component* component, scene_entity* entity) {
		if (!entity->is_scripted()) return;
		auto get = v8pp::wrap_function(r2engine::isolate(), "get_instance", [entity](v8Args args) {
			entity->mesh->get_instance_data(args);
		});
		auto set = v8pp::wrap_function(r2engine::isolate(), "set_instance", [entity](v8Args args) {
			entity->mesh->set_instance_data(args);
		});
		entity->bind(component, "instance", get, set);
	}

	void mesh_sys::bind_vertex_data(mesh_component* component, scene_entity* entity) {
		if (!entity->is_scripted()) return;
		entity->bind(component, "max_vertex_count", v8pp::wrap_function(r2engine::isolate(), nullptr, [entity](v8Args args) {
			entity->mesh->get_max_vertex_count(args);
		}), Local<Function>(), PropertyAttribute::ReadOnly);

		entity->bind(component, "vertex_count", v8pp::wrap_function(r2engine::isolate(), nullptr, [entity](v8Args args) {
			entity->mesh->get_vertex_count(args);
		}), Local<Function>(), PropertyAttribute::ReadOnly);

		entity->bind(this, "get_vertices", [](entity_system* sys, scene_entity* entity, v8Args args) {
			entity->mesh->get_vertex_data(args);
		});

		entity->bind(this, "set_vertices", [](entity_system* sys, scene_entity* entity, v8Args args) {
			entity->mesh->set_vertex_data(args);
		});
	}

	void mesh_sys::bind_index_data(mesh_component* component, scene_entity* entity) {
		if (!entity->is_scripted()) return;
		entity->bind(component, "max_index_count", v8pp::wrap_function(r2engine::isolate(), nullptr, [entity](v8Args args) {
			entity->mesh->get_max_index_count(args);
		}), Local<Function>(), PropertyAttribute::ReadOnly);

		entity->bind(component, "index_count", v8pp::wrap_function(r2engine::isolate(), nullptr, [entity](v8Args args) {
			entity->mesh->get_index_count(args);
		}), Local<Function>(), PropertyAttribute::ReadOnly);

		entity->bind(this, "get_indices", [](entity_system* sys, scene_entity* entity, v8Args args) {
			entity->mesh->get_index_data(args);
		});

		entity->bind(this, "set_indices", [](entity_system* sys, scene_entity* entity, v8Args args) {
			entity->mesh->set_index_data(args);
		});
	}

	void mesh_sys::bind_node(mesh_component* component, scene_entity* entity) {
		if (!entity->is_scripted()) return;
		auto get = v8pp::wrap_function(r2engine::isolate(), nullptr, [entity](v8Args args) {
			render_node* node = entity->mesh->get_node();
			if (!node) args.GetReturnValue().Set(v8::Null(args.GetIsolate()));
			else {
				try {
					args.GetReturnValue().Set(v8pp::convert<render_node>::to_v8(args.GetIsolate(), *node));
				} catch(std::exception& e) {
					r2Error("Failed to get node from render component");
				}
			}
		});
		auto set = v8pp::wrap_function(r2engine::isolate(), nullptr, [entity](v8Args args) {
			try {
				render_node& node = v8pp::convert<render_node>::from_v8(args.GetIsolate(), args[0]);
				entity->mesh->set_node(&node);
			} catch (std::exception e) {
				r2Error("Attempted to pass invalid value to render component's node");
			}
		});
		entity->bind(component, "node", get, set);
	}
};
