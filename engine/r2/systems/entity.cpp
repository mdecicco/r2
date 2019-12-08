#include <r2/systems/entity.h>
#include <r2/engine.h>

using namespace v8;
using namespace v8pp;

namespace r2 {
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
		r2engine::entity_created(this);
	}

	scene_entity::~scene_entity() {
		if (!m_destroyed) destroy();
		delete m_name;
		delete m_scriptFuncs;
		delete m_children;
	}

	void scene_entity::call(const mstring& function, u8 argc, LocalValueHandle* args) {
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
		if (!ensure_object_handle()) return false;

		LocalValueHandle maybeFunc = LocalObjectHandle::Cast(m_scriptObj.Get(isolate))->Get(v8str(function.c_str()));
		if (!maybeFunc->IsUndefined() && !maybeFunc->IsFunction()) {
			r2Warn("Entity \"%s\" has a \"%s\" property, but it is not a function", m_name->c_str(), function.c_str());
			return false;
		} else if (!maybeFunc->IsUndefined()) (*m_scriptFuncs)[function].Reset(isolate, LocalFunctionHandle::Cast(maybeFunc));
	}

	bool scene_entity::bind(entity_system* system, const mstring& function, void (*callback)(entity_system*, scene_entity*, v8Args)) {
		if (!ensure_object_handle()) return false;

		auto func = [system, this, callback](v8Args args) {
			callback(system, this, args);
		};

		LocalObjectHandle obj = LocalObjectHandle::Cast(m_scriptObj.Get(isolate));
		return obj->Set(v8str(function.c_str()), wrap_function(isolate, function.c_str(), func));
	}
	
	bool scene_entity::ensure_object_handle() {
		if (m_scriptObj.IsEmpty()) {
			LocalValueHandle self = convert<scene_entity>::to_v8(isolate, *this);
			if (self.IsEmpty() || self->IsNullOrUndefined()) return false;
			m_scriptObj.Reset(isolate, self);
		}
		return true;
	}

	void scene_entity::unbind(const mstring& functionOrProp) {
		auto func = m_scriptFuncs->find(functionOrProp);
		if (func != m_scriptFuncs->end()) {
			func->second.Reset();
			m_scriptFuncs->erase(functionOrProp);
			return;
		}

		if(!ensure_object_handle()) return;
		LocalObjectHandle obj = LocalObjectHandle::Cast(m_scriptObj.Get(isolate));
		auto ctx = isolate->GetCurrentContext();

		auto key = v8str(functionOrProp.c_str());

		bool deleted = obj->Delete(ctx, key).FromJust();
		if (!deleted) {
			r2Error("Failed to delete property \"%s\" on entity \"%s\"", functionOrProp.c_str(), m_name->c_str());
		}
	}

	void scene_entity::destroy() {
		if (!m_scriptObj.IsEmpty()) {
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

		for(auto fpair : *m_scriptFuncs) {
			fpair.second.Reset();
		}
		m_scriptFuncs->clear();

		stop_periodic_updates();
		destroy_periodic_update();

		r2engine::entity_destroyed(this);
		if (!m_scriptObj.IsEmpty()) {
			m_scriptObj.Reset();
			class_<scene_entity, raw_ptr_traits>::destroy_object(isolate, this);
		}
	}

	void scene_entity::deferred_destroy() {
		if (m_destroyed) {
			r2Error("Entity \"%s\" was already destroyed", m_name->c_str());
		}

		for(auto child : *m_children) child->deferred_destroy();

		m_destroyed = true;

		trace t(r2engine::isolate());
		event e(t.file, t.line, EVT_NAME_DESTROY_ENTITY, true, false);
		e.data()->write(this);

		r2engine::get()->dispatchAtFrameStart(&e);
	}

	void scene_entity::initialize() {
		initialize_periodic_update();
		start_periodic_updates();

		if (m_scriptObj.IsEmpty()) return;

		auto funcIter = m_scriptFuncs->find("wasInitialized");
		if (funcIter == m_scriptFuncs->end()) return;

		PersistentFunctionHandle func = funcIter->second;
		if (func.IsEmpty()) return;

		TryCatch tc(isolate);
		func.Get(isolate)->Call(isolate->GetCurrentContext(), m_scriptObj.Get(isolate), 0, nullptr);
		check_script_exception(isolate, tc);
	}

	void scene_entity::handle(event* evt) {
		if (evt->is_internal_only() || m_scriptObj.IsEmpty()) return;


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
		if (m_scriptObj.IsEmpty()) return;
		
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



	componentId scene_entity_component::nextComponentId = 1;
	scene_entity_component::scene_entity_component() : m_id(scene_entity_component::nextComponentId++) {
	}

	scene_entity_component::~scene_entity_component() {
	}



	entity_system_state::entity_system_state()
		: m_components(nullptr), m_newOffset(0), m_maxSize(0), m_count(0), m_componentSize(0), m_furthestComponentId(0), m_furthestComponentOffset(0) {
		m_componentOffsets = new munordered_map<componentId, size_t>();
		m_entityComponentIds = new munordered_map<entityId, componentId>();
		m_uninitializedEntities = new mvector<scene_entity*>();
	}

	entity_system_state::~entity_system_state() {
		if (m_components) delete[] m_components;
		delete m_componentOffsets;
		delete m_entityComponentIds;
		delete m_uninitializedEntities;
	}

	scene_entity_component* entity_system_state::component(componentId id) {
		auto off = m_componentOffsets->find(id);
		if (off == m_componentOffsets->end()) return nullptr;
		return (scene_entity_component*)(((u8*)m_components) + off->second);
	}

	scene_entity_component* entity_system_state::entity(entityId id) {
		auto ecomp = m_entityComponentIds->find(id);
		if (ecomp == m_entityComponentIds->end()) {
			r2Error("Failed to find entity %d in system", id);
			return nullptr;
		}

		auto off = m_componentOffsets->find(ecomp->second);
		if (off == m_componentOffsets->end()) return nullptr;
		return (scene_entity_component*)(((u8*)m_components) + off->second);
	}

	bool entity_system_state::contains_entity(entityId id) {
		return m_entityComponentIds->count(id) > 0;
	}

	void entity_system_state::destroy(entityId forEntity) {
		if (m_count == 0) return;
		for(auto comp : *m_componentOffsets) {
			printf("(%d, %d) ", comp.first, comp.second);
		}
		printf("\n");

		auto ecomp = m_entityComponentIds->find(forEntity);
		if (ecomp == m_entityComponentIds->end()) {
			r2Error("Failed to find entity %d in system, not destroying", forEntity);
			return;
		}

		auto iter = m_componentOffsets->find(ecomp->second);
		if (iter == m_componentOffsets->end()) {
			r2Error("Failed to find component %d in system, not destroying", ecomp->second);
			return;
		}

		m_entityComponentIds->erase(ecomp);

		size_t off = iter->second;
		m_componentOffsets->erase(iter);

		if (ecomp->second != m_furthestComponentId) {
			// Move the furthest component into the slot made by the destruction
			// of this component
			size_t srcOffset = m_furthestComponentOffset;
			(*m_componentOffsets)[m_furthestComponentId] = off;
			memcpy(((u8*)m_components) + off, ((u8*)m_components) + srcOffset, m_componentSize);
		}

		m_furthestComponentId = 0;
		m_furthestComponentOffset = 0;

		for(auto comp : *m_componentOffsets) {
			if (comp.second > m_furthestComponentOffset) {
				m_furthestComponentId = comp.first;
				m_furthestComponentOffset = comp.second;
			}
		}

		m_count--;
		m_newOffset -= m_componentSize;
	}

	size_t entity_system_state::allocate_offset_for_new_component() {
		if (m_newOffset == m_maxSize) {
			r2Error("Derived entity processing system does not have a high enough max component count to accomodate a new component");
			return -1;
		}

		size_t off = m_newOffset;
		m_newOffset += m_componentSize;
		return off;
	}



	entity_system_state_factory::entity_system_state_factory(entity_system* sys) : system(sys) { }

	engine_state_data* entity_system_state_factory::create() {
		auto data = new entity_system_state();
		data->m_componentSize = system->component_size();
		data->m_maxSize = system->max_component_count() * system->component_size();
		if (data->m_maxSize == 0) {
			r2Warn("Derived entity processing system returned 0 for either component size or max component count");
			return data;
		}
		data->m_components = new u8[data->m_maxSize];
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
		initialize();
	}

	void entity_system::_entity_added(scene_entity* entity) {
		m_state.enable();
		m_state->m_uninitializedEntities->push_back(entity);
		m_state.disable();
	}

	void entity_system::_entity_removed(scene_entity* entity) {
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

		m_state.disable();

		deinitialize_entity(entity);
	}

	void entity_system::initialize_entities() {
		m_state.enable();
		for (scene_entity* entity : *m_state->m_uninitializedEntities) {
			initialize_entity(entity);
		}
		m_state->m_uninitializedEntities->clear();
		m_state.disable();
	}
};