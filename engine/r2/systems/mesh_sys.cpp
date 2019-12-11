#include <r2/engine.h>
#include <r2/systems/mesh_sys.h>
#include <r2/bindings/graphics_bindings.h>

namespace r2 {
	mesh_component::mesh_component() {
	}

	mesh_component::~mesh_component() {
	}

	void mesh_component::set_node(render_node* node) {
		if (m_instance) m_instance.release();
		m_instance = node->instantiate();
	}

	render_node* mesh_component::get_node() {
		return m_instance.node();
	}

	void mesh_component::set_data(v8Args args) {
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

		m_instance.update_raw(instanceData);
		delete [] instanceData;

		get_data(args);
	}

	void mesh_component::get_data(v8Args args) {
	}


	mesh_sys::mesh_sys() {
	}

	mesh_sys::~mesh_sys() {
	}

	void mesh_sys::initialize_entity(scene_entity* entity) {
		entity->bind(this, "add_mesh_component", [](entity_system* system, scene_entity* entity, v8Args args) {
			system->addComponentTo(entity);
		});
	}

	void mesh_sys::deinitialize_entity(scene_entity* entity) {
		auto s = state();
		s.enable();
		if (!s->contains_entity(entity->id())) entity->unbind("add_mesh_component");
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
		entity->unbind("add_mesh_component");
		entity->bind(this, "remove_mesh_component", [](entity_system* system, scene_entity* entity, v8Args args) {
			system->removeComponentFrom(entity);
		});

		auto get = v8pp::wrap_function(r2engine::isolate(), nullptr, [component](v8Args args) {
			((mesh_component*)component)->get_data(args);
		});
		auto set = v8pp::wrap_function(r2engine::isolate(), nullptr, [component](v8Args args) {
			((mesh_component*)component)->set_data(args);
		});
		entity->bind(component, "instance_data", get, set);

		get = v8pp::wrap_function(r2engine::isolate(), nullptr, [component](v8Args args) {
			render_node* node = ((mesh_component*)component)->get_node();
			if (!node) args.GetReturnValue().Set(v8::Null(args.GetIsolate()));
			else {
				try {
					args.GetReturnValue().Set(v8pp::convert<render_node>::to_v8(args.GetIsolate(), *node));
				} catch(std::exception& e) {
					r2Error("Failed to get node from render component");
				}
			}
		});
		set = v8pp::wrap_function(r2engine::isolate(), nullptr, [component](v8Args args) {
			try {
				render_node& node = v8pp::convert<render_node>::from_v8(args.GetIsolate(), args[0]);
				((mesh_component*)component)->set_node(&node);
			} catch (std::exception e) {
				r2Error("Attempted to pass invalid value to render component's node");
			}
		});
		entity->bind(component, "node", get, set);
		entity->mesh = component_ref<mesh_component*>(this, component->id());
	}
	void mesh_sys::unbind(scene_entity* entity) {
		entity->unbind("node");
		entity->unbind("instance");
		entity->bind(this, "add_mesh_component", [](entity_system* system, scene_entity* entity, v8Args args) {
			system->addComponentTo(entity);
		});


		entity->mesh.clear();
	}

	void mesh_sys::initialize() {
	}

	void mesh_sys::tick(f32 dt) {
	}

	void mesh_sys::handle(event* evt) {
	}
};
