#pragma once
#include <r2/managers/memman.h>
#include <r2/managers/engine_state.h>
#include <r2/utilities/event.h>
#include <r2/bindings/v8helpers.h>
#include <r2/utilities/periodic_update.h>
#include <r2/utilities/dynamic_array.hpp>
#include <r2/bindings/math_converters.h>

namespace r2 {
	typedef u32 entityId;
	typedef u64 componentId;

	class entity_system;
	class scene_entity_component;
	class scene_entity : public event_receiver, public periodic_update {
		public:
			scene_entity(v8Args args);
			~scene_entity();

			inline entityId id() const { return m_id; }

			void call(const mstring& function, u8 argc = 0, LocalValueHandle* args = nullptr);

			// find function on entity, make it callable via scene_entity::call
			bool bind(const mstring& function);

			// bind c++ function to entity object
			bool bind(entity_system* system, const mstring& function, void (*callback)(entity_system*, scene_entity*, v8Args));
			
			template<typename T, typename U>
			bool bind(scene_entity_component* component, const mstring& prop, U T::*member, bool readonly = false) {
				if (!ensure_object_handle()) return false;

				v8::Isolate* isolate = r2engine::isolate();

				componentId compId = component->id();
				entity_system* system = component->system();

				size_t offset = (char*)&((T*)nullptr->*member) - (char*)nullptr;

				auto get = v8pp::wrap_function(isolate, nullptr, [system, compId, offset](v8Args args) {
					v8::Isolate* isolate = args.GetIsolate();
					auto state = system->state();
					state.enable();
					scene_entity_component* component = state->component(compId);
					U& prop = *(U*)(((u8*)component) + offset);
					state.disable();
					args.GetReturnValue().Set(v8pp::convert<U>::to_v8(isolate, prop));
				});

				v8::Local<v8::Function> set;
				if (!readonly) {
					set = v8pp::wrap_function(isolate, nullptr, [system, compId, offset](v8Args args) {
						v8::Isolate* isolate = args.GetIsolate();
						auto state = system->state();
						state.enable();
						scene_entity_component* component = state->component(compId);
						U& prop = *(U*)(((u8*)component) + offset);
						prop = v8pp::convert<U>::from_v8(isolate, args[0]);
						state.disable();
						args.GetReturnValue().Set(args[0]);
					});
				}

				LocalObjectHandle obj = LocalObjectHandle::Cast(m_scriptObj.Get(isolate));
				obj->SetAccessorProperty(v8str(prop.c_str()), get, set, readonly ? v8::PropertyAttribute::ReadOnly : v8::PropertyAttribute::None);
			}

			void unbind(const mstring& functionOrProp);

			inline const mstring& name() const { return *m_name; }

			void destroy();

			// this gets called from scripts to prevent mid-frame destruction of entities
			void deferred_destroy();

			inline bool destroyed() { return m_destroyed; }

			void initialize();

			virtual void handle(event* evt);

			virtual void doUpdate(f32 frameDt, f32 updateDt);

			virtual void belowFrequencyWarning(f32 percentLessThanDesired, f32 desiredFreq, f32 timeSpentLowerThanDesired);

			void add_child_entity(scene_entity* entity);
			void remove_child_entity(scene_entity* entity);
			inline scene_entity* parent() const { return m_parent; }

		private:
			bool ensure_object_handle();

			static entityId nextEntityId;
			v8::Isolate* isolate;
			PersistentValueHandle m_scriptObj;
			munordered_map<mstring, PersistentFunctionHandle>* m_scriptFuncs;
			mlist<scene_entity*>* m_children;
			scene_entity* m_parent;

			mstring* m_name;
			entityId m_id;
			bool m_destroyed;
	};

	class scene_entity_component {
		public:
			scene_entity_component();
			~scene_entity_component();

			static inline componentId nextId() { return nextComponentId; }

			inline componentId id() const { return m_id; }
			inline entity_system* system() const { return m_system; }

		private:
			friend class entity_system;
			static componentId nextComponentId;
			componentId m_id;
			entity_system* m_system;
	};

	class entity_system_state : public engine_state_data {
		public:
			entity_system_state(size_t componentSize);
			~entity_system_state();

			scene_entity_component* component(componentId id);

			scene_entity_component* entity(entityId id);

			template <typename T, typename ... construction_args>
			T* create(entityId forEntity, construction_args ... args) {
				T* comp = m_components->set<T>(scene_entity_component::nextId(), args...);
				(*m_entityComponentIds)[forEntity] = comp->id();
				return comp;
			}

			bool contains_entity(entityId id);

			void destroy(entityId forEntity);

			template <typename T>
			void for_each(void (*callback)(T*)) {
				m_components->for_each<T>(callback);
			}

		protected:
			friend class entity_system_state_factory;
			friend class entity_system;

			untyped_associative_pod_array<componentId>* m_components;
			munordered_map<entityId, componentId>* m_entityComponentIds;
			mvector<scene_entity*>* m_uninitializedEntities;
	};

	class entity_system;
	class entity_system_state_factory : public engine_state_data_factory {
		public:
			entity_system_state_factory(entity_system* sys);
			~entity_system_state_factory() { }

			virtual engine_state_data* create();

			entity_system* system;
	};

	class entity_system : public event_receiver {
		public:
			entity_system();
			~entity_system();

			virtual const size_t component_size() const = 0;
			virtual void initialize_entity(scene_entity* entity) = 0;
			virtual void deinitialize_entity(scene_entity* entity) = 0;

			void addComponentTo(scene_entity* entity);
			void removeComponentFrom(scene_entity* entity);

			virtual scene_entity_component* create_component(entityId id) = 0;

			virtual void bind(scene_entity_component* component, scene_entity* entity) = 0;
			virtual void unbind(scene_entity* entity) = 0;

			virtual void initialize() { }
			virtual void deinitialize() { }
			virtual void tick(f32 dt) { }
			virtual void handle(event* evt) { }

			inline engine_state_data_ref<entity_system_state>& state() { return m_state; }

		private:
			friend class r2engine;
			engine_state_data_ref<entity_system_state> m_state;
			void _initialize();
			void _deinitialize();
			void _entity_added(scene_entity* entity);
			void _entity_removed(scene_entity* entity);
			void initialize_entities();
	};
};