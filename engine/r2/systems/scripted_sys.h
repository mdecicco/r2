#pragma once
#include <r2/systems/entity.h>

namespace r2 {
	struct component_data {
		component_data(LocalObjectHandle& obj);

		PersistentObjectHandle handle;
	};

	class scripted_component : public scene_entity_component {
		public:
			scripted_component(LocalObjectHandle& object);
			~scripted_component();

			component_data* data;
	};

	class scripted_system_state : public engine_state_data {
		public:
			scripted_system_state(LocalObjectHandle& object);
			~scripted_system_state();

			PersistentObjectHandle data;
	};

	class scripted_sys;
	class scripted_system_state_factory : public engine_state_data_factory {
		public:
			scripted_system_state_factory(scripted_sys* sys);
			~scripted_system_state_factory();

			virtual engine_state_data* create();

			scripted_sys* system;
	};

	class scripted_sys : public entity_system, public periodic_update {
		public:
			scripted_sys(v8Args args);
			~scripted_sys();

			virtual const size_t component_size() const { return sizeof(scripted_component); }

			virtual void initialize();
			virtual void deinitialize();
			virtual void initialize_entity(scene_entity* entity);
			virtual void deinitialize_entity(scene_entity* entity);
			virtual scene_entity_component* create_component(entityId id);
			virtual void bind(scene_entity_component* component, scene_entity* entity);
			virtual void unbind(scene_entity* entity);
			virtual void tick(f32 dt);
			virtual void doUpdate(f32 frameDt, f32 updateDt);
			virtual void handle(event* evt);

			void queryComponents(v8Args args);

			LocalObjectHandle spawn_state_data();
			LocalObjectHandle spawn_component_data(entityId id);

			mstring name;
			scripted_system_state_factory* factory;
			engine_state_data_ref<scripted_system_state> scriptedState;

		protected:
			mstring m_componentScriptAccessorName;
			PersistentObjectHandle m_self;
			PersistentFunctionHandle m_stateClass;
			PersistentFunctionHandle m_compClass;
			PersistentFunctionHandle m_bindFunc;
			PersistentFunctionHandle m_unbindFunc;
			PersistentFunctionHandle m_handleFunc;
			PersistentFunctionHandle m_tickFunc;
	};
};