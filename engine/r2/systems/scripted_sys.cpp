#include <r2/systems/scripted_sys.h>
#include <r2/engine.h>
using namespace v8pp;

namespace r2 {
	component_data::component_data(LocalObjectHandle& obj) {
		Isolate* isolate = r2engine::scripts()->context()->isolate();
		handle.Reset(isolate, obj);
	}

	scripted_component::scripted_component(LocalObjectHandle& object) : data(new component_data(object)) {
	}

	scripted_component::~scripted_component() {
	}


	scripted_system_state::scripted_system_state(LocalObjectHandle& object) {
		Isolate* isolate = r2engine::scripts()->context()->isolate();
		data.Reset(isolate, object);
	}

	scripted_system_state::~scripted_system_state() {
	}


	scripted_system_state_factory::scripted_system_state_factory(scripted_sys* sys) : system(sys) {
	}

	scripted_system_state_factory::~scripted_system_state_factory() {
	}

	engine_state_data* scripted_system_state_factory::create() {
		return (engine_state_data*)new scripted_system_state(system->spawn_state_data());
	}


	scripted_sys::scripted_sys(v8Args args) : factory(nullptr) {
		if (args.Length() != 3) {
			r2Error("Script systems must be constructed with 'super(<system state class>, <component class>, <name of system>);'");
			return;
		}
		Isolate* isolate = args.GetIsolate();
		LocalObjectHandle self = args.This();

		LocalValueHandle stateClass = args[0];
		if (!stateClass->IsFunction()) {
			r2Error("System constructed with invalid state class");
			return;
		} else m_stateClass.Reset(isolate, LocalFunctionHandle::Cast(stateClass));

		LocalValueHandle compClass = args[1];
		if (!compClass->IsFunction()) {
			r2Error("System constructed with invalid component class");
			return;
		} else m_compClass.Reset(isolate, LocalFunctionHandle::Cast(compClass));

		name = convert<string>::from_v8(isolate, args[2]);

		LocalValueHandle value = self->Get(v8str("bind"));
		if (!value->IsUndefined() && !value->IsFunction()) {
			r2Error("System has a \"bind\" property that is not a function");
			return;
		} else if (!value->IsUndefined()) m_bindFunc.Reset(isolate, LocalFunctionHandle::Cast(value));
		else {
			r2Error("Classes that inherit from engine.System must have a bind function");
			return;
		}

		value = self->Get(v8str("unbind"));
		if (!value->IsUndefined() && !value->IsFunction()) {
			r2Error("System has a \"unbind\" property that is not a function");
			return;
		} else if (!value->IsUndefined()) m_unbindFunc.Reset(isolate, LocalFunctionHandle::Cast(value));
		else {
			r2Error("Classes that inherit from engine.System must have an unbind function");
			return;
		}

		value = self->Get(v8str("handleEvent"));
		if (!value->IsUndefined() && !value->IsFunction()) {
			r2Error("System has a \"handleEvent\" property that is not a function");
			return;
		} else if (!value->IsUndefined()) m_handleFunc.Reset(isolate, LocalFunctionHandle::Cast(value));
		else {
			r2Error("Classes that inherit from engine.System must have a handleEvent function");
			return;
		}

		value = self->Get(v8str("tick"));
		if (!value->IsUndefined() && !value->IsFunction()) {
			r2Error("System has a \"tick\" property that is not a function");
			return;
		} else if (!value->IsUndefined()) m_tickFunc.Reset(isolate, LocalFunctionHandle::Cast(value));
		else {
			r2Error("Classes that inherit from engine.System must have a tick function");
			return;
		}

		factory = new scripted_system_state_factory(this);
	}

	scripted_sys::~scripted_sys() {
	}

	void scripted_sys::initialize_entity(scene_entity* entity) {
		if (!entity->is_scripted()) return;
	}

	void scripted_sys::deinitialize_entity(scene_entity* entity) {
		if (!entity->is_scripted()) return;
	}

	scene_entity_component* scripted_sys::create_component(entityId id) {
		auto s = state();
		s.enable();
		auto out = s->create<scripted_component>(id, spawn_component_data());
		s.disable();
		return out;
	}

	void scripted_sys::bind(scene_entity_component* component, scene_entity* entity) {
		using c = scripted_component;
		if (entity->is_scripted()) {
			entity->unbind("add_scripted_component");
			
			// bind scripted component properties
			// entity->bind(component, "transform", &c::transform, false, true, &cascade_mat4f, "full_transform");
			
			entity->bind(this, "remove_scripted_component", [](entity_system* system, scene_entity* entity, v8Args args) {
				system->removeComponentFrom(entity);
			});
		}
	}

	void scripted_sys::unbind(scene_entity* entity) {
		if (entity->is_scripted()) {
			// unbind scripted component properties
			// entity->unbind("transform");
			entity->bind(this, "add_scripted_component", [](entity_system* system, scene_entity* entity, v8Args args) {
				system->addComponentTo(entity);
			});
		}
	}

	void scripted_sys::initialize() {
	}

	void scripted_sys::deinitialize() {
		m_stateClass.Reset();
		m_compClass.Reset();
		m_bindFunc.Reset();
		m_unbindFunc.Reset();
		m_handleFunc.Reset();
		m_tickFunc.Reset();
	}

	void scripted_sys::tick(f32 dt) {
	}

	void scripted_sys::handle(event* evt) {
	}

	LocalObjectHandle scripted_sys::spawn_state_data() {
		Isolate* isolate = r2engine::scripts()->context()->isolate();
		LocalFunctionHandle constructor = m_stateClass.Get(isolate);
		Local<Value> result;
		constructor->CallAsConstructor(isolate->GetCurrentContext(), 0, nullptr).ToLocal(&result);
		return LocalObjectHandle::Cast(result);
	}

	LocalObjectHandle scripted_sys::spawn_component_data() {
		Isolate* isolate = r2engine::scripts()->context()->isolate();
		LocalFunctionHandle constructor = m_compClass.Get(isolate);
		Local<Value> result;
		constructor->CallAsConstructor(isolate->GetCurrentContext(), 0, nullptr).ToLocal(&result);
		return LocalObjectHandle::Cast(result);
	}
};
