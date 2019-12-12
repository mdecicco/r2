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

	bool __component_exists(entity_system* sys, componentId id);
	scene_entity_component* __get_component(entity_system* sys, componentId id);

	template <typename component_ptr_type>
	class component_ref {
		public:
			component_ref() : id(0), sys(nullptr) { }
			component_ref(const component_ref& o) : id(o.id), sys(o.sys) { }
			component_ref(entity_system* _sys, componentId _id) : sys(_sys), id(_id) { }
			~component_ref() { id = 0; sys = nullptr; }

			operator bool() {
				if (id == 0 || !sys) return false;
				return __component_exists(sys, id);
			}

			component_ptr_type operator->() {
				return (component_ptr_type)__get_component(sys, id);
			}

			component_ptr_type get() {
				return (component_ptr_type)__get_component(sys, id);
			}

			void clear() {
				id = 0;
				sys = nullptr;
			}

		protected:
			friend class entity_system;

			componentId id;
			entity_system* sys;
	};

	class transform_component;
	class camera_component;
	class mesh_component;

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
			
			bool bind(scene_entity_component* component, const mstring& prop, v8::Local<v8::Function> get, v8::Local<v8::Function> set = v8::Local<v8::Function>(), v8::PropertyAttribute attribute = v8::PropertyAttribute::None);

			template<typename T, typename U, typename C = U (*)(const U&, const U&)>
			bool bind(scene_entity_component* component, const mstring& prop, U T::*member, bool readonly = false, bool cascades = false, C cascadeFunc = nullptr, const mstring& cascadedPropName = "") {
				if (!ensure_object_handle()) return false;
				if (cascades && !cascadeFunc) {
					r2Error("Property \"%s\" was specified to be cascading, but no cascade function was specified. Property will not have cascaded get accessor", prop.c_str());
					return false;
				}

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

				if (cascades) {
					get = v8pp::wrap_function(isolate, nullptr, [system, compId, member, cascadeFunc](v8Args args) {
						v8::Isolate* isolate = args.GetIsolate();
						auto state = system->state();
						state.enable();
						scene_entity_component* component = state->component(compId);
						U result = component->cascaded_property<T, U>(member, cascadeFunc);
						state.disable();
						args.GetReturnValue().Set(v8pp::convert<U>::to_v8(isolate, result));
					});

					mstring name = cascadedPropName;
					if (name.length() == 0) {
						name = prop + "_cascaded";
						r2Warn("No cascaded property name was specified for cascading property \"%s\". Using \"%s\"", prop.c_str(), name.c_str());
					}
					obj->SetAccessorProperty(v8str(name.c_str()), get, v8::Local<v8::Function>(), v8::PropertyAttribute::ReadOnly);
				}
				return true;
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

			component_ref<transform_component*> transform;
			component_ref<camera_component*> camera;
			component_ref<mesh_component*> mesh;

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

			bool contains_component(componentId id);

			void destroy(entityId forEntity);

			template <typename T>
			void for_each(void (*callback)(T*)) {
				m_components->for_each<T>(callback);
			}

			template <typename T>
			void for_each(void (*callback)(T*, size_t, bool&)) {
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

	class scene_entity_component {
		public:
			scene_entity_component();
			~scene_entity_component();

			static inline componentId nextId() { return nextComponentId; }

			inline componentId id() const { return m_id; }
			inline entity_system* system() const { return m_system; }


			/* For getting properties that are relative to the parent's same property,
			 * if one exists. Takes a pointer to the component property, and a cascade
			 * function.
			 *
			 * example:
			 * using c = transform_component;
			 * cascaded_property(&c::transform, [](const mat4f& parent, const mat4f& child) { return parent * child; });
			 */
			template <typename T, typename U, typename C>
			U cascaded_property(U T::*member, C cascade) {
				size_t offset = (char*)&((T*)nullptr->*member) - (char*)nullptr;
				U& thisProp = *(U*)(((u8*)this) + offset);
				scene_entity* parent = m_entity->parent();
				if (parent) {
					auto state = m_system->state();
					state.enable();
					if (!state->contains_entity(parent->id())) {
						state.disable();
						return thisProp;
					}
					auto parentComp = state->entity(parent->id());
					U parentProp = parentComp->cascaded_property<T, U, C>(member, cascade);
					state.disable();
					return cascade(parentProp, thisProp);
				}
				return thisProp;
			}

		private:
			friend class entity_system;
			static componentId nextComponentId;

			componentId m_id;
			entity_system* m_system;
			scene_entity* m_entity;
		};
};