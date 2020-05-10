#include <r2/systems/entity.h>
#include <r2/systems/scripted_sys.h>
#include <r2/engine.h>

using namespace v8;
using namespace v8pp;

namespace r2 {
	bool __component_exists(entity_system* sys, componentId id) {
		auto state = sys->state();
		state.enable();
		bool valid = state->contains_component(id);
		state.disable();
		return valid;
	}

	scene_entity_component* __get_component(entity_system* sys, componentId id) {
		auto state = sys->state();
		state.enable();
		auto comp = state->component(id);
		state.disable();
		return comp;
	}


	entityId scene_entity::nextEntityId = 1;
	scene_entity::scene_entity(v8Args args)
		: m_id(scene_entity::nextEntityId++), m_name(nullptr), m_scriptFuncs(nullptr), m_destroyed(false), m_parent(nullptr), m_children(nullptr) {
		isolate = args.GetIsolate();
		if (args.Length() != 1) {
			r2Warn("No name parameter passed to entity constructor");
		} else {
			if (!args[0]->IsString()) {
				r2Warn("Parameter passed to entity constructor is not a string (should be the name of the entity)");
			} else {
				m_name = new mstring(convert<mstring>::from_v8(isolate, args[0]));
				if (m_name->length() == 0) {
					r2Warn("Entity names should not be empty.");
				}
			}
		}
		if (!m_name) {
			char num[16] = { 0 };
			snprintf(num, 16, "%lu", m_id);
			m_name = new mstring(mstring("entity_") + num);
		}

		m_scriptFuncs = new munordered_map<mstring, PersistentFunctionHandle>();
		m_children = new mlist<scene_entity*>();
		m_propInterpolation = new munordered_map<mstring, prop_interpolate_info>();
		m_propAnimation = new munordered_map<mstring, prop_animate_info>();

		initialize_periodic_update();
		initialize_event_receiver();
		r2engine::entity_created(this);

		m_scripted = true;
		m_doesUpdate = false;
	}

	scene_entity::scene_entity(const mstring& name)
		: m_id(scene_entity::nextEntityId++), m_name(nullptr), m_scriptFuncs(nullptr), m_destroyed(false), m_parent(nullptr), m_children(nullptr), isolate(nullptr) {
		m_name = new mstring(name);
		m_scriptFuncs = new munordered_map<mstring, PersistentFunctionHandle>();
		m_children = new mlist<scene_entity*>();
		m_propInterpolation = new munordered_map<mstring, prop_interpolate_info>();
		m_propAnimation = new munordered_map<mstring, prop_animate_info>();

		initialize_periodic_update();
		initialize_event_receiver();
		r2engine::entity_created(this);
		
		m_scripted = true;
		m_doesUpdate = false;

		isolate = r2engine::isolate();
		Local<Object> obj = class_<scene_entity, v8pp::raw_ptr_traits>::reference_external(isolate, this);
		m_scriptObj.Reset(isolate, obj);
	}

	scene_entity::~scene_entity() {
		if (!m_destroyed) deferred_destroy();
		delete m_name;
		m_name = nullptr;
		if (m_scriptFuncs) {
			delete m_scriptFuncs;
			m_scriptFuncs = nullptr;
		}
		delete m_children;
		m_children = nullptr;

		for (auto it = m_propAnimation->begin();it != m_propAnimation->end();it++) {
			delete it->second.propAccess;
		}

		delete m_propInterpolation;
		m_propInterpolation = nullptr;
		delete m_propAnimation;
		m_propAnimation = nullptr;
	}

	Local<Object> scene_entity::script_obj() const {
		if (m_scriptObj.IsEmpty()) return Local<Object>::Cast(Null(isolate));
		return Local<Object>::Cast(m_scriptObj.Get(isolate));
	}

	void scene_entity::call(const mstring& function, u8 argc, LocalValueHandle* args) {
		if (!m_scripted) return;
		if (m_scriptObj.IsEmpty()) return;

		auto funcIter = m_scriptFuncs->find(function);
		if (funcIter == m_scriptFuncs->end()) {
			r2Error("Function \"%s\" was never defined on entity \"%s\"", function.c_str(), m_name->c_str());
			return;
		}

		PersistentFunctionHandle func = funcIter->second;
		if (func.IsEmpty()) return;

		TryCatch tc(isolate);
		func.Get(isolate)->Call(isolate->GetCurrentContext(), m_scriptObj.Get(isolate), argc, args);
		check_script_exception(isolate, tc);
	}

	bool scene_entity::bind(const mstring& function) {
		if (!m_scripted) return false;
		if (!ensure_object_handle()) return false;

		LocalValueHandle maybeFunc = LocalObjectHandle::Cast(m_scriptObj.Get(isolate))->Get(v8str(function.c_str()));
		if (!maybeFunc->IsUndefined() && !maybeFunc->IsFunction()) {
			r2Warn("Entity \"%s\" has a \"%s\" property, but it is not a function", m_name->c_str(), function.c_str());
			return false;
		} else if (!maybeFunc->IsUndefined()) {
			if (function == "update") m_doesUpdate = true;
			(*m_scriptFuncs)[function].Reset(isolate, LocalFunctionHandle::Cast(maybeFunc));
			return true;
		}

		return false;
	}

	bool scene_entity::bind(entity_system* system, const mstring& function, void (*callback)(entity_system*, scene_entity*, v8Args)) {
		if (!m_scripted) return false;
		if (!ensure_object_handle()) return false;

		auto func = [system, this, callback](v8Args args) {
			callback(system, this, args);
		};

		LocalObjectHandle obj = LocalObjectHandle::Cast(m_scriptObj.Get(isolate));
		return obj->Set(v8str(function.c_str()), wrap_function(isolate, function.c_str(), func));
	}

	bool scene_entity::bind(entity_system* system, const mstring& parentPropName, const mstring& function, void (*callback)(entity_system*, scene_entity*, v8Args)) {
		if (!m_scripted) return false;
		if (!ensure_object_handle()) return false;

		auto func = [system, this, callback](v8Args args) {
			callback(system, this, args);
		};

		LocalObjectHandle obj = LocalObjectHandle::Cast(m_scriptObj.Get(isolate));

		Local<Value> parentObjVal = obj->Get(v8str(parentPropName.c_str()));
		if (parentObjVal.IsEmpty() || parentObjVal->IsNullOrUndefined()) {
			parentObjVal = Object::New(isolate);
			obj->Set(v8str(parentPropName.c_str()), parentObjVal);
		}
		LocalObjectHandle parentObj = LocalObjectHandle::Cast(parentObjVal);

		return parentObj->Set(v8str(function.c_str()), wrap_function(isolate, function.c_str(), func));
	}

	bool scene_entity::bind(scene_entity_component* component, const mstring& prop, v8::Local<v8::Function> get, v8::Local<v8::Function> set, v8::PropertyAttribute attribute) {
		if (!m_scripted) return false;
		if (!ensure_object_handle()) return false;

		v8::Isolate* isolate = r2engine::isolate();

		LocalObjectHandle obj = LocalObjectHandle::Cast(m_scriptObj.Get(isolate));
		obj->SetAccessorProperty(v8str(prop.c_str()), get, set, attribute);

		return true;
	}

	bool scene_entity::bind(scene_entity_component* component, const mstring& parentPropName, const mstring& prop, v8::Local<v8::Function> get, v8::Local<v8::Function> set, v8::PropertyAttribute attribute) {
		if (!m_scripted) return false;
		if (!ensure_object_handle()) return false;

		v8::Isolate* isolate = r2engine::isolate();

		LocalObjectHandle obj = LocalObjectHandle::Cast(m_scriptObj.Get(isolate));

		Local<Value> parentObjVal = obj->Get(v8str(parentPropName.c_str()));
		if (parentObjVal.IsEmpty() || parentObjVal->IsNullOrUndefined()) {
			parentObjVal = Object::New(isolate);
			obj->Set(v8str(parentPropName.c_str()), parentObjVal);
		}
		LocalObjectHandle parentObj = LocalObjectHandle::Cast(parentObjVal);

		parentObj->SetAccessorProperty(v8str(prop.c_str()), get, set, attribute);

		return true;
	}
	
	bool scene_entity::ensure_object_handle() {
		if (!m_scripted) return false;
		if (m_scriptObj.IsEmpty()) {
			LocalValueHandle self = convert<scene_entity>::to_v8(isolate, *this);
			if (self.IsEmpty() || self->IsNullOrUndefined()) return false;
			m_scriptObj.Reset(isolate, self);
		}
		return true;
	}

	void scene_entity::unbind(const mstring& functionOrProp) {
		if (!m_scripted) return;
		auto func = m_scriptFuncs->find(functionOrProp);
		if (func != m_scriptFuncs->end()) {
			func->second.Reset();
			m_scriptFuncs->erase(functionOrProp);
			if (functionOrProp == "update") m_doesUpdate = false;
			return;
		}

		if(!ensure_object_handle()) return;
		LocalObjectHandle obj = LocalObjectHandle::Cast(m_scriptObj.Get(isolate));
		auto ctx = isolate->GetCurrentContext();

		auto key = v8str(functionOrProp.c_str());

		bool deleted = obj->Delete(ctx, key).FromJust();
		if (!deleted) {
			r2Error("Failed to delete property \"%s\" on entity \"%s\"", functionOrProp.c_str(), m_name->c_str());
		} else {
			auto pi_it = m_propInterpolation->find(functionOrProp);
			if (pi_it != m_propInterpolation->end()) m_propInterpolation->erase(pi_it);
			
			auto pa_it = m_propAnimation->find(functionOrProp);
			if (pa_it != m_propAnimation->end()) {
				delete pa_it->second.propAccess;
				m_propAnimation->erase(pa_it);
			}
		}
	}

	void scene_entity::deferred_destroy() {
		m_destroyed = true;

		willBeDestroyed();

		if (m_scripted && !m_scriptObj.IsEmpty()) {
			auto funcIter = m_scriptFuncs->find("willBeDestroyed");
			if (funcIter != m_scriptFuncs->end()) {
				PersistentFunctionHandle func = funcIter->second;
				if (!func.IsEmpty()) {
					TryCatch tc(isolate);
					func.Get(isolate)->Call(isolate->GetCurrentContext(), m_scriptObj.Get(isolate), 0, nullptr);
					check_script_exception(isolate, tc);
				}
			}
		}

		r2engine::entity_destroyed(this);

		stop_periodic_updates();
		destroy_periodic_update();
		destroy_event_receiver();

		if (m_scripted) {
			auto funcs = *m_scriptFuncs;
			for(auto fpair : funcs) {
				fpair.second.Reset();

				if (!m_scriptObj.IsEmpty()) {
					// just in case the entity still exists in JS, unbind all C++ functions and component property accessors
					LocalObjectHandle obj = LocalObjectHandle::Cast(m_scriptObj.Get(isolate));
					auto ctx = isolate->GetCurrentContext();

					auto key = v8str(fpair.first.c_str());

					bool deleted = obj->Delete(ctx, key).FromJust();
					if (!deleted) {
						r2Error("Failed to delete property \"%s\" on entity \"%s\"", fpair.first.c_str(), m_name->c_str());
					}
				}
			}
			m_scriptFuncs->clear();

			if (!m_scriptObj.IsEmpty()) {
				m_scriptObj.Reset();
				class_<scene_entity, raw_ptr_traits>::destroy_object(isolate, this);
			}
		}
	}

	void scene_entity::set_name(const mstring& name) {
		delete m_name;
		m_name = new mstring(name);
	}

	void scene_entity::destroy() {
		if (m_destroyed) {
			r2Error("Entity \"%s\" was already destroyed", m_name->c_str());
		}


		for(auto child : *m_children) child->deferred_destroy();

		m_destroyed = true;

		// todo: make this first condition only happen if called from JS
		//if (m_scripted) {
			//trace t(r2engine::isolate());
			//event e(t.file, t.line, EVT_NAME_DESTROY_ENTITY, true, false);
			//e.data()->write(this);
			//r2engine::get()->dispatchAtFrameStart(&e);
		//} else {
			event e = evt(EVT_NAME_DESTROY_ENTITY, true, false);
			e.data()->write(this);
			r2engine::get()->dispatchAtFrameStart(&e);
		//}
	}

	void scene_entity::initialize() {
		start_periodic_updates();

		onInitialize();

		if (!m_scripted) return;
		if (m_scriptObj.IsEmpty()) return;

		bind("handleEvent");
		bind("update");
		bind("willBeDestroyed");
		if (bind("wasInitialized")) call("wasInitialized");
	}

	void scene_entity::updatesStarted() {
		if (m_doesUpdate) return;
		m_doesUpdate = true;

		event e = evt(EVT_NAME_ENABLE_ENTITY_UPDATES, true, false);
		e.data()->write(this);
		r2engine::get()->dispatchAtFrameStart(&e);
	}

	void scene_entity::updatesStopped() {
		if (!m_doesUpdate) return;
		m_doesUpdate = false;

		event e = evt(EVT_NAME_DISABLE_ENTITY_UPDATES, true, false);
		e.data()->write(this);
		r2engine::get()->dispatchAtFrameStart(&e);
	}

	void scene_entity::handle(event* evt) {
		if (evt->is_internal_only()) return;

		onEvent(evt);
		
		if(!m_scripted || m_scriptObj.IsEmpty()) return;

		auto funcIter = m_scriptFuncs->find("handleEvent");
		if (funcIter == m_scriptFuncs->end()) return;

		PersistentFunctionHandle func = funcIter->second;
		if (func.IsEmpty()) return;

		TryCatch tc(isolate);
		Local<Value> param = Local<Value>::Cast(convert<event>::to_v8(isolate, *evt));
		func.Get(isolate)->Call(isolate->GetCurrentContext(), m_scriptObj.Get(isolate), 1, &param);
		check_script_exception(isolate, tc);
	}

	void scene_entity::doUpdate(f32 frameDt, f32 updateDt) {
		onUpdate(frameDt, updateDt);

		if (!m_scripted || m_scriptObj.IsEmpty()) return;
		
		auto funcIter = m_scriptFuncs->find("update");
		if (funcIter == m_scriptFuncs->end()) return;

		PersistentFunctionHandle func = funcIter->second;
		if (func.IsEmpty()) return;

		TryCatch tc(isolate);
		
		Local<Value> param[2] = {
			to_v8(isolate, frameDt),
			to_v8(isolate, updateDt)
		};

		func.Get(isolate)->Call(isolate->GetCurrentContext(), m_scriptObj.Get(isolate), 2, param);
		check_script_exception(isolate, tc);
	}

	void scene_entity::belowFrequencyWarning(f32 percentLessThanDesired, f32 desiredFreq, f32 timeSpentLowerThanDesired) {
		r2Warn("Entity \"%s\" has been updating at %0.2f%% less than the desired frequency (%0.2f Hz) for more than %0.2f seconds", m_name->c_str(), percentLessThanDesired, desiredFreq, timeSpentLowerThanDesired);
	}

	void scene_entity::add_child_entity(scene_entity* entity) {
		if (entity->m_parent) entity->m_parent->remove_child_entity(entity);
		entity->m_parent = this;
		m_children->push_front(entity);
		add_child(entity);
		r2engine::get()->remove_child(entity);
	}

	void scene_entity::remove_child_entity(scene_entity* entity) {
		if (entity->m_parent != this) {
			r2Error("Entity \"%s\" is not a child of entity \"%s\"", entity->m_name->c_str(), m_name->c_str());
			return;
		}

		for (auto i = m_children->begin();i != m_children->end();i++) {
			if ((*i) == entity) {
				m_children->erase(i);
				break;
			}
		}

		entity->m_parent = nullptr;
		remove_child(entity);
		r2engine::get()->add_child(entity);
	}

	void scene_entity::add_custom_component(const mstring& systemName) {
		scripted_sys* sys = r2engine::scripted_system(systemName);
		sys->addComponentTo(this);
	}

	void scene_entity::remove_custom_component(const mstring& systemName) {
		scripted_sys* sys = r2engine::scripted_system(systemName);
		sys->removeComponentFrom(this);
	}

	void scene_entity::set_interpolation(const mstring& prop, interpolate::interpolation_transition_mode transition, f32 duration) {
		auto it = m_propInterpolation->find(prop);
		if (it != m_propInterpolation->end()) {
			it->second.mode = transition;
			it->second.duration = duration;
		} else {
			r2Error("Entity '%s' has no interpolatable property '%s'", m_name->c_str(), prop.c_str());
		}
	}

	void scene_entity::animatable_props(mvector<mstring>& out) const {
		for (auto it = m_propAnimation->begin();it != m_propAnimation->end();it++) {
			out.push_back(it->first);
		}
	}

	animation_track_base* scene_entity::animate_prop(const mstring& prop, animation_group* animation) {
		auto it = m_propAnimation->find(prop);
		if (it == m_propAnimation->end()) {
			r2Error("Entity '%s' has no animatable property named '%s'", m_name->c_str(), prop.c_str());
			return nullptr;
		}

		auto& pai = it->second;
		return pai.add_to_anim(prop, pai.propAccess, animation);
	}

	keyframe_base* scene_entity::create_keyframe(const mstring& prop, animation_group* animation, interpolate::interpolation_transition_mode mode) {
		auto it = m_propAnimation->find(prop);
		if (it == m_propAnimation->end()) {
			r2Error("Entity '%s' has no animatable property named '%s'", m_name->c_str(), prop.c_str());
			return nullptr;
		}

		auto& pai = it->second;
		return pai.create_keyframe(prop, pai.propAccess, animation, mode);
	}

	mvector<scene_entity*> scene_entity::children() {
		mvector<scene_entity*> result;
		mlist<scene_entity*>& l = *m_children;
		for (scene_entity* e : l) {
			result.push_back(e);
		}
		return result;
	}



	componentId scene_entity_component::nextComponentId = 1;
	scene_entity_component::scene_entity_component() : m_id(scene_entity_component::nextComponentId++) {
	}

	scene_entity_component::~scene_entity_component() {
	}



	entity_system_state::entity_system_state(size_t componentSize) {
		m_components = new untyped_associative_pod_array<componentId>(componentSize);
		m_entityComponentIds = new munordered_map<entityId, componentId>();
		m_uninitializedEntities = new mvector<scene_entity*>();
	}

	entity_system_state::~entity_system_state() {
		delete m_components;
		delete m_entityComponentIds;
		delete m_uninitializedEntities;
	}

	scene_entity_component* entity_system_state::component(componentId id) {
		return (scene_entity_component*)m_components->get(id);
	}

	scene_entity_component* entity_system_state::entity(entityId id) {
		auto ecomp = m_entityComponentIds->find(id);
		if (ecomp == m_entityComponentIds->end()) {
			r2Error("Failed to find entity %d in system", id);
			return nullptr;
		}

		return (scene_entity_component*)m_components->get(ecomp->second);
	}

	bool entity_system_state::contains_entity(entityId id) {
		return m_entityComponentIds->count(id) > 0;
	}

	bool entity_system_state::contains_component(componentId id) {
		return m_components->has(id);
	}

	void entity_system_state::destroy(entityId forEntity) {
		auto ecomp = m_entityComponentIds->find(forEntity);
		if (ecomp == m_entityComponentIds->end()) {
			r2Error("Failed to find entity %d in system", forEntity);
			return;
		}

		scene_entity_component* comp = (scene_entity_component*)m_components->get(ecomp->second);
		m_components->remove(ecomp->second);
		m_entityComponentIds->erase(ecomp);
		comp->destroy();
	}



	entity_system_state_factory::entity_system_state_factory(entity_system* sys) : system(sys) { }

	engine_state_data* entity_system_state_factory::create() {
		auto data = new entity_system_state(system->component_size());
		return data;
	}



	entity_system::entity_system() {
	}

	entity_system::~entity_system() {
	}

	void entity_system::addComponentTo(scene_entity* entity) {
		m_state.enable();
		if (m_state->contains_entity(entity->id())) {
			r2Warn("Entity \"%s\" already has a component of this type", entity->name().c_str());
			m_state.disable();
			return;
		}
		scene_entity_component* comp = create_component(entity->id());
		comp->m_system = this;
		comp->m_entity = entity;
		bind(comp, entity);
		m_state.disable();
	}

	void entity_system::removeComponentFrom(scene_entity* entity) {
		m_state.enable();
		if (!m_state->contains_entity(entity->id())) {
			r2Warn("Entity \"%s\" does not have a component of this type", entity->name().c_str());
			m_state.disable();
			return;
		}
		unbind(entity);
		m_state->destroy(entity->id());
		m_state.disable();
	}

	void entity_system::_initialize() {
		auto fac = new entity_system_state_factory(this);
		auto stateMgr = r2engine::get()->states();
		m_state = stateMgr->register_state_data_factory<entity_system_state>(fac);
		initialize_event_receiver();
		initialize();
	}

	void entity_system::_deinitialize() {
		destroy_event_receiver();
		deinitialize();
	}

	void entity_system::_entity_added(scene_entity* entity) {
		/*
		if (!r2engine::started()) {
			initialize_entity(entity);
			return;
		}
		*/

		m_state.enable();
		m_state->m_uninitializedEntities->push_back(entity);
		m_state.disable();
	}

	void entity_system::_entity_removed(scene_entity* entity) {
		/*
		if (!r2engine::started()) {
			m_state.enable();
			if (m_state->contains_entity(entity->id())) {
				unbind(entity);
				m_state->destroy(entity->id());
			}
			deinitialize_entity(entity);
			m_state.disable();

			return;
		}
		*/

		m_state.enable();
		for (auto it = m_state->m_uninitializedEntities->begin(); it != m_state->m_uninitializedEntities->end(); it++) {
			if ((*it)->id() == entity->id()) {
				m_state->m_uninitializedEntities->erase(it);
				break;
			}
		}

		if (m_state->contains_entity(entity->id())) {
			unbind(entity);
			m_state->destroy(entity->id());
		}

		deinitialize_entity(entity);
		m_state.disable();
	}

	void entity_system::initialize_entities() {
		m_state.enable();
		auto& entities = *m_state->m_uninitializedEntities;
		for (scene_entity* entity : entities) {
			initialize_entity(entity);
		}
		m_state->m_uninitializedEntities->clear();
		m_state.disable();
	}
};