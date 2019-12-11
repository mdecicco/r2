#include <r2/engine.h>
#include <r2/managers/drivers/gl/driver.h>

#include <v8pp/context.hpp>
#include <v8pp/class.hpp>
#include <v8pp/convert.hpp>
#include <v8pp/module.hpp>

#include <r2/bindings/math_converters.h>

using namespace v8;
using namespace v8pp;

namespace r2 {
	template <typename T>
	void copy_attr_value(void* dest, u16& offset, Isolate* i, Local<Value>& value) {
		T val = convert<T>::from_v8(i, value);
		memcpy((u8*)dest + offset, &val, sizeof(T));
		offset += sizeof(T);
	}

	bool parse_vertex(Local<Object>& obj, vertex_format* fmt, void* dest) {
		Isolate* isolate = obj->GetIsolate();
		auto properties = obj->GetPropertyNames(isolate->GetCurrentContext());
		Local<Value> propsValue;
		if(!properties.ToLocal(&propsValue)) return false;

		Local<Array> propArr = Local<Array>::Cast(propsValue);
		auto attributeTypes = fmt->attributes();
		if (propArr->Length() != attributeTypes.size()) {
			r2Warn("Vertex object contains more properties than the vertex format.");
			return false;
		}

		#define attrCase(sv, t) case sv: { copy_attr_value<t>(dest, offset, isolate, obj->Get(propArr->Get(i))); break; }
		try {
			u16 offset = 0;
			for(u8 i = 0;i < propArr->Length();i++) {
				vertex_attribute_type t = attributeTypes[i];
				switch(t) {
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
						return false;
					}
				}
			}
		} catch (std::exception e) {
			return false;
		}
		#undef attrCase

		return true;
	}

	bool parse_index(Local<Value>& value, index_type type, void* dest) {
		u16 offset = 0;
		Isolate* isolate = r2engine::get()->scripts()->context()->isolate();
		switch(type) {
		case it_unsigned_byte: {
			copy_attr_value<u8>(dest, offset, isolate, value);
			break;
		}
		case it_unsigned_short: {
			copy_attr_value<u16>(dest, offset, isolate, value);
			break;
		}
		case it_unsigned_int: {
			copy_attr_value<u32>(dest, offset, isolate, value);
			break;
		}
		default: {
			r2Warn("Invalid index type specified to MeshInfo.");
			return false;
		}
		}

		return true;
	}
	
	bool parse_instance(Local<Object>& obj, instance_format* fmt, void* dest) {
		Isolate* isolate = obj->GetIsolate();
		auto properties = obj->GetPropertyNames(isolate->GetCurrentContext());
		Local<Value> propsValue;
		if(!properties.ToLocal(&propsValue)) return false;

		Local<Array> propArr = Local<Array>::Cast(propsValue);
		auto attributeTypes = fmt->attributes();
		if (propArr->Length() != attributeTypes.size()) {
			r2Warn("Instance object contains more properties than the instance format.");
			return false;
		}

		#define attrCase(sv, t) case sv: { copy_attr_value<t>(dest, offset, isolate, obj->Get(propArr->Get(i))); break; }
		try {
			u16 offset = 0;
			for(u8 i = 0;i < propArr->Length();i++) {
				instance_attribute_type t = attributeTypes[i];
				switch(t) {
					attrCase(iat_int, i32)
						attrCase(iat_float, f32)
						attrCase(iat_uint, u32)
						attrCase(iat_vec2i, vec2i)
						attrCase(iat_vec2f, vec2f)
						attrCase(iat_vec2ui, vec2ui)
						attrCase(iat_vec3i, vec3i)
						attrCase(iat_vec3f, vec3f)
						attrCase(iat_vec3ui, vec3ui)
						attrCase(iat_vec4i, vec4i)
						attrCase(iat_vec4f, vec4f)
						attrCase(iat_vec4ui, vec4ui)
						attrCase(iat_mat2i, mat2i)
						attrCase(iat_mat2f, mat2f)
						attrCase(iat_mat2ui, mat2ui)
						attrCase(iat_mat3i, mat3i)
						attrCase(iat_mat3f, mat3f)
						attrCase(iat_mat3ui, mat3ui)
						attrCase(iat_mat4i, mat4i)
						attrCase(iat_mat4f, mat4f)
						attrCase(iat_mat4ui, mat4ui)
				default: {
						r2Warn("Invalid instance attribute type specified to instance format.");
						return false;
					}
				}
			}
		} catch (std::exception e) {
			return false;
		}
		#undef attrCase

		return true;
	}

	bool parse_vertices(v8Args args, vertex_format* fmt, u8** dest, size_t* count) {
		*dest = nullptr;
		*count = 0;

		mvector<Local<Object>> maybeVertices;
		for(u8 a = 0;a < args.Length();a++) {
			Local<Value> arg = args[a];
			if (arg->IsArray()) {
				Local<Array> arr = Local<Array>::Cast(arg);
				for(u32 v = 0;v < arr->Length();v++) {
					Local<Value> maybeVertex = arr->Get(v);
					if (maybeVertex->IsObject()) {
						maybeVertices.push_back(Local<Object>::Cast(maybeVertex));
					} else {
						r2Error("Invalid parameter: Expected a variable number of arguments, that can either be arrays of vertex objects, or vertex objects.");
						return false;
					}
				}
			} else if (arg->IsObject()) maybeVertices.push_back(Local<Object>::Cast(arg));
			else {
				r2Error("Invalid parameter: Expected a variable number of arguments, that can either be arrays of vertex objects, or vertex objects.");
				return false;
			}
		}

		*dest = new u8[maybeVertices.size() * fmt->size()];
		*count = maybeVertices.size();
		::memset(*dest, 0, maybeVertices.size() * fmt->size());

		u32 idx = 0;
		for (auto maybeVertex : maybeVertices) {
			if (!parse_vertex(maybeVertex, fmt, (*dest) + (idx * fmt->size()))) {
				r2Error("Invalid Parameter: A vertex object was encountered that does not adhere to the specified vertex format:");
				r2Warn("{%s}", fmt->to_string().c_str());
				delete[] (*dest);
				*dest = nullptr;
				*count = 0;
				return false;
			}
			idx++;
		}

		return true;
	}

	bool parse_indices(v8Args args, index_type type, u8** dest, size_t* count) {
		*dest = nullptr;
		*count = 0;

		mvector<Local<Value>> maybeIndices;
		for(u8 a = 0;a < args.Length();a++) {
			Local<Value> arg = args[a];
			if (arg->IsArray()) {
				Local<Array> arr = Local<Array>::Cast(arg);
				for(u32 v = 0;v < arr->Length();v++) {
					Local<Value> maybeIndex = arr->Get(v);
					if (maybeIndex->IsNumber()) {
						maybeIndices.push_back(maybeIndex);
					} else {
						r2Error("Invalid parameter: MeshInfo.appendIndices accepts a variable number of arguments, that can either be arrays of index values, or index values.");
						return false;
					}
				}
			} else if (arg->IsObject()) maybeIndices.push_back(arg);
			else {
				r2Error("Invalid parameter: MeshInfo.appendIndices accepts a variable number of arguments, that can either be arrays of index values, or index values.");
				return false;
			}
		}

		*dest = new u8[maybeIndices.size() * type];
		*count = maybeIndices.size();
		::memset(*dest, 0, maybeIndices.size() * type);

		u32 idx = 0;
		for (auto maybeIndex : maybeIndices) {
			if (!parse_index(maybeIndex, type, (*dest) + (idx * (size_t)type))) {
				r2Error("An instance object was passed to MeshInfo.appendIndices that does not adhere to the index type.");
				delete[] (*dest);
				*dest = nullptr;
				*count = 0;
				return false;
			}
			idx++;
		}

		return true;
	}

	void mesh_construction_data::append_vertices_v8(v8Args args) {
		if (!m_vertexFormat) {
			r2Error("MeshInfo does not have an vertex format set, you can't append vertices to it");
			return;
		}

		if (m_last_vertex_idx == m_vertex_count) {
			r2Error("Cannot append vertices to MeshInfo, the MeshInfo's maximum vertex count (%d) has been reached. Resize with MeshInfo.max_vertices", m_vertex_count);
			return;
		}

		u8* vdata = nullptr;
		size_t count = 0;
		if (parse_vertices(args, m_vertexFormat, &vdata, &count)) {
			if (count + m_last_vertex_idx > m_vertex_count) {
				r2Error("Cannot append %d vertices to MeshInfo, max vertex count needs to be increased.", count);
				delete[] vdata;
				return;
			}

			size_t vsize = m_vertexFormat->size();
			for(size_t i = 0;i < count;i++) {
				append_vertex_data(vdata + (i * vsize));
			}

			delete[] vdata;
		}
	}

	void mesh_construction_data::append_indices_v8(v8Args args) {
		if (m_last_index_idx == m_index_count) {
			r2Error("Cannot append indices to MeshInfo, the MeshInfo's maximum index count (%d) has been reached. Resize with MeshInfo.max_indices", m_index_count);
			return;
		}

		u8* idata = nullptr;
		size_t count = 0;
		if (parse_indices(args, m_indexType, &idata, &count)) {
			if (count + m_last_index_idx > m_index_count) {
				r2Error("Cannot append %d indices to MeshInfo, max index count needs to be increased.", count);
				delete[] idata;
				return;
			}

			for(size_t i = 0;i < count;i++) {
				append_index_data(idata + (i * (size_t)m_indexType));
			}

			delete[] idata;
		}
	}

	void mesh_construction_data::append_instances_v8(v8Args args) {
		if (!m_instanceFormat) {
			r2Error("MeshInfo does not have an instance format set, you can't append instances to it");
			return;
		}

		if (m_last_instance_idx == m_instance_count) {
			r2Error("Cannot append instances to MeshInfo, the MeshInfo's maximum instance count (%d) has been reached. Resize with MeshInfo.max_instances", m_instance_count);
			return;
		}

		mvector<Local<Object>> maybeInstances;
		for(u8 a = 0;a < args.Length();a++) {
			Local<Value> arg = args[a];
			if (arg->IsArray()) {
				Local<Array> arr = Local<Array>::Cast(arg);
				for(u32 v = 0;v < arr->Length();v++) {
					Local<Value> maybeInstance = arr->Get(v);
					if (maybeInstance->IsObject()) {
						maybeInstances.push_back(Local<Object>::Cast(maybeInstance));
					} else {
						r2Error("Invalid parameter: MeshInfo.appendInstances accepts a variable number of arguments, that can either be arrays of instance objects, or instance objects.");
						return;
					}
				}
			} else if (arg->IsObject()) maybeInstances.push_back(Local<Object>::Cast(arg));
			else {
				r2Error("Invalid parameter: MeshInfo.appendInstances accepts a variable number of arguments, that can either be arrays of instance objects, or instance objects.");
				return;
			}
		}

		u8* idata = new u8[maybeInstances.size() * m_instanceFormat->size()];
		memset(idata, 0, maybeInstances.size() * m_instanceFormat->size());

		u32 idx = 0;
		for (auto maybeInstance : maybeInstances) {
			if (!parse_instance(maybeInstance, m_instanceFormat, idata + (idx * m_instanceFormat->size()))) {
				r2Error("An instance object was passed to MeshInfo.appendInstances that does not adhere to the specified instance format:");
				r2Warn("{%s}", m_instanceFormat->to_string().c_str());
				delete[] idata;
				return;
			}
			idx++;
		}

		if (idx + m_last_instance_idx > m_instance_count) {
			r2Error("Cannot append %d instances to MeshInfo, max instance count needs to be increased.", idx);
			delete[] idata;
			return;
		}

		size_t isize = m_instanceFormat->size();
		for(idx = 0;idx < maybeInstances.size();idx++) {
			append_instance_data(idata + (idx * isize));
		}

		delete[] idata;
	}

	void instantiate_material(v8Args args) {
		Isolate* isolate = args.GetIsolate();
		node_material& mat = convert<node_material>::from_v8(isolate, args.This());
		node_material_instance* instance = mat.instantiate(r2engine::get()->current_scene());
		
		args.GetReturnValue().Set(class_<node_material_instance, v8pp::raw_ptr_traits>::reference_external(isolate, instance));
	}



	enum jsDriverType { dt_opengl = 0 };

	void set_driver(jsDriverType t) {
		r2engine* eng = r2engine::get();
		if (eng->renderer()->driver()) {
			r2Error("A render driver has already been specified.");
			return;
		}

		switch(t) {
			case dt_opengl:
				eng->renderer()->set_driver(new gl_render_driver(eng->renderer()));
				break;
			default:
				r2Error("Invalid renderer type specified");
		}
	}

	void load_shader(v8Args args) {
		Isolate* isolate = r2engine::get()->scripts()->context()->isolate();
		mstring assetName = convert<mstring>::from_v8(isolate, args[0]);
		mstring file = convert<mstring>::from_v8(isolate, args[1]);

		shader_program* shader = r2engine::get()->current_scene()->load_shader(file, assetName);

		args.GetReturnValue().Set(class_<shader_program, v8pp::raw_ptr_traits>::reference_external(isolate, shader));
	}

	void build_render_node(v8Args args) {
		Isolate* i = args.GetIsolate();
		try {
			mesh_construction_data& mesh = convert<mesh_construction_data>::from_v8(args.GetIsolate(), args.This());
			scene* s = r2engine::get()->current_scene();
			render_node* n = s->add_mesh(&mesh);
			
			args.GetReturnValue().Set(class_<render_node, v8pp::raw_ptr_traits>::reference_external(i, n));
		} catch (std::exception e) {
			r2Error("Invalid parameter specified to MeshInfo.makeRenderable.");
		}
	}



	void bind_enums(module& mod) {
		// index_type
		{
			module m(mod.isolate());
			m.set_const("ubyte", it_unsigned_byte);
			m.set_const("ushort", it_unsigned_short);
			m.set_const("uint", it_unsigned_int);
			mod.set("IndexType", m);
		}

		// vertex_attribute_type
		{
			module m(mod.isolate());
			m.set_const("int", vat_int);
			m.set_const("float", vat_float);
			m.set_const("uint", vat_uint);
			m.set_const("vec2i", vat_vec2i);
			m.set_const("vec2f", vat_vec2f);
			m.set_const("vec2ui", vat_vec2ui);
			m.set_const("vec3i", vat_vec3i);
			m.set_const("vec3f", vat_vec3f);
			m.set_const("vec3ui", vat_vec3ui);
			m.set_const("vec4i", vat_vec4i);
			m.set_const("vec4f", vat_vec4f);
			m.set_const("vec4ui", vat_vec4ui);
			m.set_const("mat2i", vat_mat2i);
			m.set_const("mat2f", vat_mat2f);
			m.set_const("mat2ui", vat_mat2ui);
			m.set_const("mat3i", vat_mat3i);
			m.set_const("mat3f", vat_mat3f);
			m.set_const("mat3ui", vat_mat3ui);
			m.set_const("mat4i", vat_mat4i);
			m.set_const("mat4f", vat_mat4f);
			m.set_const("mat4ui", vat_mat4ui);
			mod.set("VertexAttrType", m);
		}

		// instance_attribute_type
		{
			module m(mod.isolate());
			m.set_const("int", iat_int);
			m.set_const("float", iat_float);
			m.set_const("byte", iat_uint);
			m.set_const("vec2i", iat_vec2i);
			m.set_const("vec2f", iat_vec2f);
			m.set_const("vec2b", iat_vec2ui);
			m.set_const("vec3i", iat_vec3i);
			m.set_const("vec3f", iat_vec3f);
			m.set_const("vec3b", iat_vec3ui);
			m.set_const("vec4i", iat_vec4i);
			m.set_const("vec4f", iat_vec4f);
			m.set_const("vec4b", iat_vec4ui);
			m.set_const("mat2i", iat_mat2i);
			m.set_const("mat2f", iat_mat2f);
			m.set_const("mat2b", iat_mat2ui);
			m.set_const("mat3i", iat_mat3i);
			m.set_const("mat3f", iat_mat3f);
			m.set_const("mat3b", iat_mat3ui);
			m.set_const("mat4i", iat_mat4i);
			m.set_const("mat4f", iat_mat4f);
			m.set_const("mat4b", iat_mat4ui);
			mod.set("InstanceAttrType", m);
		}

		// uniform_attribute_type
		{
			module m(mod.isolate());
			m.set_const("int", uat_int);
			m.set_const("uint", uat_uint);
			m.set_const("float", uat_float);
			m.set_const("vec2i", uat_vec2i);
			m.set_const("vec2ui", uat_vec2ui);
			m.set_const("vec2f", uat_vec2f);
			m.set_const("vec3i", uat_vec3i);
			m.set_const("vec3ui", uat_vec3ui);
			m.set_const("vec3f", uat_vec3f);
			m.set_const("vec4i", uat_vec4i);
			m.set_const("vec4ui", uat_vec4ui);
			m.set_const("vec4f", uat_vec4f);
			m.set_const("mat2i", uat_mat2i);
			m.set_const("mat2ui", uat_mat2ui);
			m.set_const("mat2f", uat_mat2f);
			m.set_const("mat3i", uat_mat3i);
			m.set_const("mat3ui", uat_mat3ui);
			m.set_const("mat3f", uat_mat3f);
			m.set_const("mat4i", uat_mat4i);
			m.set_const("mat4ui", uat_mat4ui);
			m.set_const("mat4f", uat_mat4f);
			mod.set("UniformAttrType", m);
		}

		// driver types
		{
			module m(mod.isolate());
			m.set_const("OpenGL", dt_opengl);
			mod.set("RenderDriver", m);
		}
	}

	void bind_classes(module& mod) {
		// vertex_format
		{
			class_<vertex_format, v8pp::raw_ptr_traits> s(mod.isolate());
			register_class_state(s);
			s.ctor<>();
			s.set("addAttr", &vertex_format::add_attr);
			s.set("attributes", property(&vertex_format::attributes));
			s.set("size", property(&vertex_format::size));
			s.set("offsetOf", &vertex_format::offsetOf);
			s.set("toString", &vertex_format::to_string);
			s.set("toHashString", &vertex_format::hash_name);
			mod.set("VertexFormat", s);
		}

		// instance_format
		{
			class_<instance_format, v8pp::raw_ptr_traits> s(mod.isolate());
			register_class_state(s);
			s.ctor<>();
			s.set("addAttr", &instance_format::add_attr);
			s.set("attributes", property(&instance_format::attributes));
			s.set("size", property(&instance_format::size));
			s.set("offsetOf", &instance_format::offsetOf);
			s.set("toString", &instance_format::to_string);
			s.set("toHashString", &instance_format::hash_name);
			mod.set("InstanceFormat", s);
		}

		// uniform_format
		{
			class_<uniform_format, v8pp::raw_ptr_traits> s(mod.isolate());
			register_class_state(s);
			s.auto_wrap_objects(true);
			s.ctor<>();
			s.set("addAttr", &uniform_format::add_attr);
			s.set("attributes", property(&uniform_format::attributes));
			s.set("attributeNames", property(&uniform_format::attributeNames));
			s.set("size", property(&uniform_format::size));
			s.set("offsetOf", &uniform_format::offsetOf);
			s.set("toString", &uniform_format::to_string);
			s.set("toHashString", &uniform_format::hash_name);
			mod.set("UniformFormat", s);
		}

		// uniform_block
		{
			class_<uniform_block, v8pp::raw_ptr_traits> s(mod.isolate());
			register_class_state(s);
			s.auto_wrap_objects(true);
			s.set("name", property(&uniform_block::name));
			s.set("int", &uniform_block::uniform_int);
			s.set("uint", &uniform_block::uniform_uint);
			s.set("float", &uniform_block::uniform_float);
			s.set("vec2i", &uniform_block::uniform_vec2i);
			s.set("vec2ui", &uniform_block::uniform_vec2ui);
			s.set("vec2f", &uniform_block::uniform_vec2f);
			s.set("vec3i", &uniform_block::uniform_vec3i);
			s.set("vec3ui", &uniform_block::uniform_vec3ui);
			s.set("vec3f", &uniform_block::uniform_vec3f);
			s.set("vec4i", &uniform_block::uniform_vec4i);
			s.set("vec4ui", &uniform_block::uniform_vec4ui);
			s.set("vec4f", &uniform_block::uniform_vec4f);
			s.set("mat2i", &uniform_block::uniform_mat2i);
			s.set("mat2ui", &uniform_block::uniform_mat2ui);
			s.set("mat2f", &uniform_block::uniform_mat2f);
			s.set("mat3i", &uniform_block::uniform_mat3i);
			s.set("mat3ui", &uniform_block::uniform_mat3ui);
			s.set("mat3f", &uniform_block::uniform_mat3f);
			s.set("mat4i", &uniform_block::uniform_mat4i);
			s.set("mat4ui", &uniform_block::uniform_mat4ui);
			s.set("mat4f", &uniform_block::uniform_mat4f);
			mod.set("UniformBlock", s);
		}

		// render_node
		{
			class_<render_node, v8pp::raw_ptr_traits> s(mod.isolate());
			register_class_state(s);
			s.auto_wrap_objects(true);
			s.set("material_instance", property(&render_node::material_instance, &render_node::set_material_instance));
			mod.set("RenderNode", s);
		}

		// shader_program
		{
			class_<shader_program, v8pp::raw_ptr_traits> s(mod.isolate());
			register_class_state(s);
			s.auto_wrap_objects(true);
			mod.set("ShaderProgram", s);
		}

		// node_material
		{
			class_<node_material, v8pp::raw_ptr_traits> s(mod.isolate());
			register_class_state(s);
			s.ctor<const mstring&, uniform_format*>();
			s.set("shader", property(&node_material::shader, &node_material::set_shader));
			s.set("format", property(&node_material::format));
			s.set("instantiate", &instantiate_material);
			s.set("name", &node_material::name);
			mod.set("Material", s);
		}

		// node_material_instance
		{
			class_<node_material_instance, v8pp::raw_ptr_traits> s(mod.isolate());
			register_class_state(s);
			s.auto_wrap_objects(true);
			s.set("material", property(&node_material_instance::material));
			s.set("uniforms", property(&node_material_instance::uniforms_v8));
			mod.set("MaterialInstance", s);
		}

		// mesh_construction_data
		{
			using c = mesh_construction_data;
			class_<c, v8pp::raw_ptr_traits> s(mod.isolate());
			register_class_state(s);
			s.ctor<vertex_format*, index_type, instance_format*>();
			s.set("max_vertices", property(&c::max_vertex_count, &c::set_max_vertex_count));
			s.set("vertex_count", property(&c::vertex_count));
			s.set("max_indices", property(&c::max_index_count, &c::set_max_index_count));
			s.set("index_count", property(&c::index_count));
			s.set("max_instances", property(&c::max_instance_count, &c::set_max_instance_count));
			s.set("instance_count", property(&c::instance_count));
			s.set("vertex_format", property(&c::vertexFormat));
			s.set("index_type", property(&c::indexType));
			s.set("instance_format", property(&c::instanceFormat));
			s.set("appendVertices", &c::append_vertices_v8);
			s.set("appendIndices", &c::append_indices_v8);
			s.set("appendInstances", &c::append_instances_v8);
			s.set("makeRenderable", &build_render_node);

			mod.set("MeshInfo", s);
		}

	}

	void bind_functions(module& mod) {
		mod.set("set_driver", &set_driver);
		mod.set("load_shader", &load_shader);
	}

	void bind_graphics(context* ctx) {
		auto isolate = ctx->isolate();

		module m(isolate);

		bind_enums(m);
		bind_classes(m);
		bind_functions(m);

		isolate->GetCurrentContext()->Global()->Set(v8str("gfx"), m.new_instance());
	}

	void release_graphics_objects() {
		Isolate* i = r2engine::get()->scripts()->context()->isolate();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<mesh_construction_data>()).remove_objects();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<node_material_instance>()).remove_objects();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<node_material>()).remove_objects();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<shader_program>()).remove_objects();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<render_node>()).remove_objects();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<uniform_block>()).remove_objects();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<uniform_format>()).remove_objects();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<instance_format>()).remove_objects();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<vertex_format>()).remove_objects();
	}
	
	void reset_graphics_object_storage() {
		Isolate* i = r2engine::get()->scripts()->context()->isolate();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<mesh_construction_data>()).reset_objects_map();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<node_material_instance>()).reset_objects_map();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<node_material>()).reset_objects_map();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<shader_program>()).reset_objects_map();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<render_node>()).reset_objects_map();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<uniform_block>()).reset_objects_map();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<uniform_format>()).reset_objects_map();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<instance_format>()).reset_objects_map();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<vertex_format>()).reset_objects_map();
	}
};