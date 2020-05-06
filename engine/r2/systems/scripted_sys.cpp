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
		if (args.Length() != 3 && args.Length() != 4) {
			r2Error("Script systems must be constructed with 'super(<system state class>, <component class>, <name of system>);'");
			return;
		}
		Isolate* isolate = args.GetIsolate();
		LocalObjectHandle self = args.This();

		name = convert<string>::from_v8(isolate, args[0]);
		m_componentScriptAccessorName = name;

		LocalValueHandle stateClass = args[1];
		if (!stateClass->IsFunction()) {
			r2Error("System constructed with invalid state class");
			return;
		} else m_stateClass.Reset(isolate, LocalFunctionHandle::Cast(stateClass));

		LocalValueHandle compClass = args[2];
		if (!compClass->IsFunction()) {
			r2Error("System constructed with invalid component class");
			return;
		} else m_compClass.Reset(isolate, LocalFunctionHandle::Cast(compClass));

		if (args.Length() == 4) {
			m_componentScriptAccessorName = convert<string>::from_v8(isolate, args[3]);
		}

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
		auto out = s->create<scripted_component>(id, spawn_component_data(id));
		s.disable();
		return out;
	}

	void scripted_sys::bind(scene_entity_component* component, scene_entity* entity) {
		if (entity->is_scripted()) {
			// bind scripted component properties
			// entity->bind(component, "transform", &c::transform, false, true, &cascade_mat4f, "full_transform");

			Isolate* isolate = r2engine::scripts()->context()->isolate();
			auto get = v8pp::wrap_function(isolate, nullptr, [component](v8Args args) {
				args.GetReturnValue().Set(((scripted_component*)component)->data->handle.Get(args.GetIsolate()));
			});

			entity->bind(component, m_componentScriptAccessorName, get);

			TryCatch tc(isolate);
			auto param = Local<Value>::Cast(convert<scene_entity*>::to_v8(isolate, entity));
			m_bindFunc.Get(isolate)->Call(isolate->GetCurrentContext(), m_self.Get(isolate), 1, &param);
			check_script_exception(isolate, tc);
		}
	}

	void scripted_sys::unbind(scene_entity* entity) {
		if (entity->is_scripted()) {
			// unbind scripted component properties
			Isolate* isolate = r2engine::scripts()->context()->isolate();
			TryCatch tc(isolate);
			auto param = Local<Value>::Cast(convert<scene_entity*>::to_v8(isolate, entity));
			m_unbindFunc.Get(isolate)->Call(isolate->GetCurrentContext(), m_self.Get(isolate), 1, &param);
			check_script_exception(isolate, tc);

			entity->unbind(m_componentScriptAccessorName);
		}
	}

	void scripted_sys::initialize() {
		initialize_periodic_update();
		initialize_event_receiver();
		start_periodic_updates();

		Isolate* isolate = r2engine::scripts()->context()->isolate();
		Local<Object> self = convert<scripted_sys>::to_v8(isolate, *this);
		m_self.Reset(isolate, self);

		auto get_state = v8pp::wrap_function(isolate, nullptr, [this](v8Args args) {
			this->scriptedState.enable();
			args.GetReturnValue().Set(this->scriptedState->data.Get(args.GetIsolate()));
			this->scriptedState.disable();
		});
		self->SetAccessorProperty(v8str("state"), get_state);
	}

	void scripted_sys::deinitialize() {
		m_self.Reset();
		m_stateClass.Reset();
		m_compClass.Reset();
		m_bindFunc.Reset();
		m_unbindFunc.Reset();
		m_handleFunc.Reset();
		m_tickFunc.Reset();

		destroy_event_receiver();
		destroy_periodic_update();
	}

	void scripted_sys::tick(f32 dt) {
		update(dt);
	}

	void scripted_sys::doUpdate(f32 frameDt, f32 updateDt) {
		Isolate* isolate = r2engine::scripts()->context()->isolate();
		TryCatch tc(isolate);
		Local<Value> params[2] = {
			Local<Value>::Cast(convert<f32>::to_v8(isolate, frameDt)),
			Local<Value>::Cast(convert<f32>::to_v8(isolate, updateDt))
		};
		m_tickFunc.Get(isolate)->Call(isolate->GetCurrentContext(), m_self.Get(isolate), 2, params);
		check_script_exception(isolate, tc);
	}

	void scripted_sys::handle(event* evt) {
		Isolate* isolate = r2engine::scripts()->context()->isolate();
		TryCatch tc(isolate);
		auto param = Local<Value>::Cast(convert<event>::to_v8(isolate, *evt));
		m_handleFunc.Get(isolate)->Call(isolate->GetCurrentContext(), m_self.Get(isolate), 1, &param);
		check_script_exception(isolate, tc);
	}

	void scripted_sys::queryComponents(v8Args args) {
		Isolate* isolate = args.GetIsolate();
		EscapableHandleScope scope(isolate);
		
		mvector<scripted_component*> results;

		auto& s = state();
		s.enable();
		s->for_each<scripted_component>([&results](scripted_component* comp) {
			// todo: filters
			results.push_back(comp);
			return true;
		});
		s.disable();

		Local<Array> arr = Array::New(isolate, results.size());
		for (size_t i = 0;i < results.size();i++) {
			scripted_component* comp = results[i];
			arr->Set(i, comp->data->handle.Get(isolate));
		}

		args.GetReturnValue().Set(scope.Escape(arr));
	}

	LocalObjectHandle scripted_sys::spawn_state_data() {
		Isolate* isolate = r2engine::scripts()->context()->isolate();
		LocalFunctionHandle constructor = m_stateClass.Get(isolate);
		Local<Value> result;
		constructor->CallAsConstructor(isolate->GetCurrentContext(), 0, nullptr).ToLocal(&result);
		return LocalObjectHandle::Cast(result);
	}

	LocalObjectHandle scripted_sys::spawn_component_data(entityId id) {
		Isolate* isolate = r2engine::scripts()->context()->isolate();
		LocalFunctionHandle constructor = m_compClass.Get(isolate);
		Local<Value> result;
		constructor->CallAsConstructor(isolate->GetCurrentContext(), 0, nullptr).ToLocal(&result);
		LocalObjectHandle object = LocalObjectHandle::Cast(result);

		auto get_entity = v8pp::wrap_function(isolate, nullptr, [this, id](v8Args args) {
			auto& s = this->state();
			s.enable();
			args.GetReturnValue().Set(Local<Value>::Cast(convert<scene_entity*>::to_v8(args.GetIsolate(), s->entity(id)->entity())));
			s.disable();
		});
		auto get_id = v8pp::wrap_function(isolate, nullptr, [this, id](v8Args args) {
			auto& s = this->state();
			s.enable();
			args.GetReturnValue().Set(Local<Value>::Cast(convert<componentId>::to_v8(args.GetIsolate(), s->entity(id)->id())));
			s.disable();
		});
		object->SetAccessorProperty(v8str("entity"), get_entity);
		object->SetAccessorProperty(v8str("id"), get_id);

		return object;
	}
};
