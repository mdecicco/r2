#pragma once
#include <r2/managers/memman.h>
#include <r2/managers/engine_state.h>
#include <r2/utilities/event.h>
#include <r2/bindings/v8helpers.h>
#include <r2/utilities/periodic_update.h>
#include <r2/utilities/dynamic_array.hpp>
#include <r2/bindings/math_converters.h>
#include <r2/systems/animation.h>

namespace r2 {
	typedef u32 entityId;
	typedef u64 componentId;

	class entity_system;
	class scene_entity_component;
	class entity_system_state;

	bool __component_exists(entity_system* sys, componentId id);
	scene_entity_component* __get_component(entity_system* sys, componentId id);

	template <typename component_ptr_type>
	class component_ref {
		public:
			component_ref() : id(0), state(nullptr) { }
			component_ref(const component_ref& o) : id(o.id), state(o.state) { }
			component_ref(entity_system* sys, componentId _id) : state(nullptr), id(_id) {
				auto s = sys->state();
				s.enable();
				state = s.get();
				s.disable();
			}
			~component_ref() { id = 0; state = nullptr; }

			operator bool();

			component_ptr_type operator->();

			component_ptr_type get();

			void clear() {
				id = 0;
				state = nullptr;
			}

		protected:
			friend class entity_system;
			friend class mesh_sys;
			friend class transform_sys;
			friend class camera_sys;

			componentId id;
			entity_system_state* state;
	};

	class transform_component;
	class camera_component;
	class mesh_component;
	class physics_component;
	class lighting_component;
	class animation_component;

	struct animatable_property_data {
		component_ref<scene_entity_component*> ref;
		size_t offset;
	};

	template <typename T>
	inline void animation_property_setter(const T& value, scene_entity* entity, void* user_data) {
		animatable_property_data* propAccess = (animatable_property_data*)user_data;
		scene_entity_component* comp = propAccess->ref.get();
		if (comp) *((T*)(((u8*)comp) + propAccess->offset)) = value;
	}

	template <typename T>
	inline animation_track_base* bind_property_to_animation(const mstring& prop, animatable_property_data* propAccess, animation_group* anim) {
		return anim->add_track<T>(prop, *(T*)(((u8*)propAccess->ref.get()) + propAccess->offset), animation_property_setter<T>, default_interpolator<T>, propAccess);
	}
	
	template <typename T>
	inline keyframe_base* create_property_keyframe(const mstring& prop, animatable_property_data* propAccess, animation_group* anim, interpolate::interpolation_transition_mode mode) {
		animation_track<T>* track = anim->track<T>(prop);
		if (!track) return nullptr;
		return track->set(*(T*)(((u8*)propAccess->ref.get()) + propAccess->offset), anim->current_time(), mode);
	}

	class scene_entity : public event_receiver, public periodic_update {
		public:
			scene_entity(v8Args args);
			scene_entity(const mstring& name);
			virtual ~scene_entity();

			/* Accessors */
			inline entityId id() const { return m_id; }
			inline const mstring& name() const { return *m_name; }
			inline bool destroyed() const { return m_destroyed; }
			inline bool is_scripted() const { return m_scripted; }
			inline scene_entity* parent() const { return m_parent; }
			inline bool doesUpdate() const { return m_doesUpdate; }
			inline bool has_children() const { return m_children->size() > 0; }
			Local<Object> script_obj() const;

			/* Functions for scripted entities */
			void call(const mstring& function, u8 argc = 0, LocalValueHandle* args = nullptr);
			bool bind(const mstring& function); // find function on entity, make it callable via scene_entity::call
			bool bind(entity_system* system, const mstring& function, void (*callback)(entity_system*, scene_entity*, v8Args)); // bind c++ function to entity object
			bool bind(entity_system* system, const mstring& parentPropName, const mstring& function, void (*callback)(entity_system*, scene_entity*, v8Args)); // bind c++ function to entity object
			bool bind(scene_entity_component* component, const mstring& prop, v8::Local<v8::Function> get, v8::Local<v8::Function> set = v8::Local<v8::Function>(), v8::PropertyAttribute attribute = v8::PropertyAttribute::None);
			bool bind(scene_entity_component* component, const mstring& parentPropName, const mstring& prop, v8::Local<v8::Function> get, v8::Local<v8::Function> set = v8::Local<v8::Function>(), v8::PropertyAttribute attribute = v8::PropertyAttribute::None);

			template<typename T, typename U, typename C = U (*)(const U&, const U&)>
			bool bind(scene_entity_component* component, const mstring& prop, U T::*member, bool readonly = false, bool cascades = false, C cascadeFunc = nullptr, const mstring& cascadedPropName = "") {
				if (!m_scripted) return false;
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
			
			template<typename T, typename U, typename C = U (*)(const U&, const U&)>
			bool bind(scene_entity_component* component, const mstring& parentPropName, const mstring& prop, U T::*member, bool readonly = false, bool cascades = false, C cascadeFunc = nullptr, const mstring& cascadedPropName = "") {
				if (!m_scripted) return false;
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

				Local<Value> parentObjVal = obj->Get(v8str(parentPropName.c_str()));
				if (parentObjVal.IsEmpty() || parentObjVal->IsNullOrUndefined()) {
					parentObjVal = Object::New(isolate);
					obj->Set(v8str(parentPropName.c_str()), parentObjVal);
				}
				LocalObjectHandle parentObj = LocalObjectHandle::Cast(parentObjVal);

				parentObj->SetAccessorProperty(v8str(prop.c_str()), get, set, readonly ? v8::PropertyAttribute::ReadOnly : v8::PropertyAttribute::None);

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
					parentObj->SetAccessorProperty(v8str(name.c_str()), get, v8::Local<v8::Function>(), v8::PropertyAttribute::ReadOnly);
				}
				return true;
			}

			template<typename T, typename U, typename C = U (*)(const U&, const U&)>
			bool bind_interpolatable(scene_entity_component* component, const mstring& prop, U T::*member, bool cascades = false, C cascadeFunc = nullptr, const mstring& cascadedPropName = "") {
				if (!m_scripted) return false;
				if (!ensure_object_handle()) return false;
				if (cascades && !cascadeFunc) {
					r2Error("Property \"%s\" was specified to be cascading, but no cascade function was specified. Property will not have cascaded get accessor", prop.c_str());
					return false;
				}

				componentId compId = component->id();
				entity_system* system = component->system();
				size_t offset = (char*)&((T*)nullptr->*member) - (char*)nullptr;
				(*m_propInterpolation)[prop] = prop_interpolate_info({ interpolate::itm_none, 0.0f });

				v8::Isolate* isolate = r2engine::isolate();

				(*m_propAnimation)[prop] = prop_animate_info({
					new animatable_property_data({
						component_ref<scene_entity_component*>(system, compId),
						offset
					}),
					bind_property_to_animation<U>,
					create_property_keyframe<U>
				});

				auto get = v8pp::wrap_function(isolate, nullptr, [system, compId, offset](v8Args args) {
					v8::Isolate* isolate = args.GetIsolate();
					auto state = system->state();
					state.enable();
					scene_entity_component* component = state->component(compId);
					U& prop = *(U*)(((u8*)component) + offset);
					state.disable();
					args.GetReturnValue().Set(v8pp::convert<U>::to_v8(isolate, prop));
				});

				v8::Local<v8::Function> set = v8pp::wrap_function(isolate, nullptr, [this, prop, system, compId, offset](v8Args args) {
					v8::Isolate* isolate = args.GetIsolate();
					auto state = system->state();
					state.enable();
					scene_entity_component* component = state->component(compId);
					prop_interpolate_info& interp = m_propInterpolation->at(prop);
					// Note: this is dangerous
					// If the host component array resizes before the animation is complete
					// then the (U*)(((u8*)component) + offset) pointer will be invalidated
					r2engine::interpolation()->animate((U*)(((u8*)component) + offset), v8pp::convert<U>::from_v8(isolate, args[0]), interp.duration, interpolate::from_enum(interp.mode));
					state.disable();
					args.GetReturnValue().Set(args[0]);
				});

				LocalObjectHandle obj = LocalObjectHandle::Cast(m_scriptObj.Get(isolate));
				obj->SetAccessorProperty(v8str(prop.c_str()), get, set, v8::PropertyAttribute::None);

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

			template<typename T, typename U, typename C = U (*)(const U&, const U&)>
			bool bind_interpolatable(scene_entity_component* component, const mstring& parentPropName, const mstring& prop, U T::*member, bool cascades = false, C cascadeFunc = nullptr, const mstring& cascadedPropName = "") {
				if (!m_scripted) return false;
				if (!ensure_object_handle()) return false;
				if (cascades && !cascadeFunc) {
					r2Error("Property \"%s\" was specified to be cascading, but no cascade function was specified. Property will not have cascaded get accessor", prop.c_str());
					return false;
				}

				mstring propPath = parentPropName + "." + prop;
				(*m_propInterpolation)[propPath] = prop_interpolate_info({ interpolate::itm_none, 0.0f });


				componentId compId = component->id();
				entity_system* system = component->system();
				size_t offset = (char*)&((T*)nullptr->*member) - (char*)nullptr;
				(*m_propAnimation)[propPath] = prop_animate_info({
					new animatable_property_data({
						component_ref<scene_entity_component*>(system, compId),
						offset
					}),
					bind_property_to_animation<U>,
					create_property_keyframe<U>
				});

				v8::Isolate* isolate = r2engine::isolate();

				auto get = v8pp::wrap_function(isolate, nullptr, [system, compId, offset](v8Args args) {
					v8::Isolate* isolate = args.GetIsolate();
					auto state = system->state();
					state.enable();
					scene_entity_component* component = state->component(compId);
					U& prop = *(U*)(((u8*)component) + offset);
					state.disable();
					args.GetReturnValue().Set(v8pp::convert<U>::to_v8(isolate, prop));
				});

				v8::Local<v8::Function> set = v8pp::wrap_function(isolate, nullptr, [this, propPath, system, compId, offset](v8Args args) {
					v8::Isolate* isolate = args.GetIsolate();
					auto state = system->state();
					state.enable();
					scene_entity_component* component = state->component(compId);
					prop_interpolate_info& interp = m_propInterpolation->at(propPath);
					// Note: this is dangerous
					// If the host component array resizes before the animation is complete
					// then the (U*)(((u8*)component) + offset) pointer will be invalidated
					r2engine::interpolation()->animate((U*)(((u8*)component) + offset), v8pp::convert<U>::from_v8(isolate, args[0]), interp.duration, interpolate::from_enum(interp.mode));
					state.disable();
					args.GetReturnValue().Set(args[0]);
				});

				LocalObjectHandle obj = LocalObjectHandle::Cast(m_scriptObj.Get(isolate));

				Local<Value> parentObjVal = obj->Get(v8str(parentPropName.c_str()));
				if (parentObjVal.IsEmpty() || parentObjVal->IsNullOrUndefined()) {
					parentObjVal = Object::New(isolate);
					obj->Set(v8str(parentPropName.c_str()), parentObjVal);
				}
				LocalObjectHandle parentObj = LocalObjectHandle::Cast(parentObjVal);

				parentObj->SetAccessorProperty(v8str(prop.c_str()), get, set, v8::PropertyAttribute::None);

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
					parentObj->SetAccessorProperty(v8str(name.c_str()), get, v8::Local<v8::Function>(), v8::PropertyAttribute::ReadOnly);
				}
				return true;
			}

			template <typename F>
			bool bind(const mstring& parentPropName, const mstring& prop, F&& func) {
				if (!m_scripted) return false;
				if (!ensure_object_handle()) return false;
				LocalObjectHandle obj = LocalObjectHandle::Cast(m_scriptObj.Get(isolate));

				Local<Value> parentObjVal = obj->Get(v8str(parentPropName.c_str()));
				if (parentObjVal.IsEmpty() || parentObjVal->IsNullOrUndefined()) {
					parentObjVal = Object::New(isolate);
					obj->Set(v8str(parentPropName.c_str()), parentObjVal);
				}

				LocalObjectHandle parentObj = LocalObjectHandle::Cast(parentObjVal);

				parentObj->Set(v8str(prop.c_str()), v8pp::wrap_function(isolate, prop.c_str(), func));
				return true;
			}

			template <typename F>
			bool bind(const mstring& prop, F&& func) {
				if (!m_scripted) return false;
				if (!ensure_object_handle()) return false;
				LocalObjectHandle obj = LocalObjectHandle::Cast(m_scriptObj.Get(isolate));
				using F_type = typename std::decay<F>::type;
				obj->Set(v8str(prop.c_str()), v8pp::wrap_function(isolate, prop.c_str(), std::forward<F_type>(func)));
				return true;
			}

			void unbind(const mstring& functionOrProp);

			/* Functions for classes deriving from this */
			virtual void onInitialize() { }
			virtual void onUpdate(f32 frameDt, f32 updateDt) { }
			virtual void onEvent(event* evt) { }
			virtual void willBeDestroyed() { }
			
			/* Functions for both */
			void set_name(const mstring& name);
			void destroy();
			void add_child_entity(scene_entity* entity);
			void remove_child_entity(scene_entity* entity);
			void add_custom_component(const mstring& systemName);
			void remove_custom_component(const mstring& systemName);
			void set_interpolation(const mstring& prop, interpolate::interpolation_transition_mode transition, f32 duration);
			void animatable_props(mvector<mstring>& out) const;
			animation_track_base* animate_prop(const mstring& prop, animation_group* animation);
			keyframe_base* create_keyframe(const mstring& prop, animation_group* animation, interpolate::interpolation_transition_mode mode);
			mvector<scene_entity*> children();
			
			template <typename F>
			void for_each_child(F&& callback) {
				auto& children = *m_children;
				for (scene_entity* child : children) {
					callback(child);
				}
			}
	
			/* For convenience */
			component_ref<transform_component*> transform;
			component_ref<camera_component*> camera;
			component_ref<mesh_component*> mesh;
			component_ref<physics_component*> physics;
			component_ref<lighting_component*> lighting;
			component_ref<animation_component*> animation;

		private:
			friend class r2engine;
			friend class entity_system;

			bool ensure_object_handle();
			void initialize();
			void deferred_destroy();

			virtual void updatesStarted();
			virtual void updatesStopped();
			virtual void handle(event* evt);
			virtual void doUpdate(f32 frameDt, f32 updateDt);
			virtual void belowFrequencyWarning(f32 percentLessThanDesired, f32 desiredFreq, f32 timeSpentLowerThanDesired);

			struct prop_interpolate_info {
				interpolate::interpolation_transition_mode mode;
				f32 duration;
			};

			struct prop_animate_info {
				animatable_property_data* propAccess;
				animation_track_base* (*add_to_anim)(const mstring& /*prop*/, animatable_property_data* /*propAccess*/, animation_group* /*anim*/);
				keyframe_base* (*create_keyframe)(const mstring& /*prop*/, animatable_property_data* /*propAccess*/, animation_group* /*anim*/, interpolate::interpolation_transition_mode /*mode*/);
			};

			static entityId nextEntityId;
			v8::Isolate* isolate;
			PersistentValueHandle m_scriptObj;
			munordered_map<mstring, PersistentFunctionHandle>* m_scriptFuncs;
			munordered_map<mstring, prop_interpolate_info>* m_propInterpolation;
			munordered_map<mstring, prop_animate_info>* m_propAnimation;
			mlist<scene_entity*>* m_children;
			scene_entity* m_parent;

			mstring* m_name;
			entityId m_id;
			bool m_destroyed;
			bool m_scripted;
			bool m_doesUpdate;
	};

	class entity_system_state : public engine_state_data {
		public:
			entity_system_state(size_t componentSize);
			~entity_system_state();

			scene_entity_component* component(componentId id);

			scene_entity_component* entity(entityId id);

			template <typename T, typename ... construction_args>
			T* create(entityId forEntity, construction_args ... args) {
				T* comp = m_components->construct<T>(scene_entity_component::nextId(), args...);
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
			
			template <typename T, typename F>
			void for_each(F&& callback) {
				using F_type = typename std::decay<F>::type;
				m_components->for_each<T, F>(std::forward<F_type>(callback));
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

			virtual void destroy() { }

			static inline componentId nextId() { return nextComponentId; }

			inline componentId id() const { return m_id; }
			inline entity_system* system() const { return m_system; }
			inline scene_entity* entity() const { return m_entity; }

			virtual bool serialize(data_container* out) { return false; }
			virtual bool deserialize(data_container* in) { return false; }


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

	template <typename component_ptr_type>
	component_ptr_type component_ref<component_ptr_type>::get() {
		if (id == 0 || !state) return nullptr;
		return (component_ptr_type)state->component(id);
	}
	template <typename component_ptr_type>
	component_ptr_type component_ref<component_ptr_type>::operator->() {
		if (id == 0 || !state) return nullptr;
		return (component_ptr_type)state->component(id);
	}
	template <typename component_ptr_type>
	component_ref<component_ptr_type>::operator bool() {
		if (id == 0 || !state) return false;
		return state->contains_component(id);
	}
};