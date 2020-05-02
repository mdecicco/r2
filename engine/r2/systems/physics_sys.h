#pragma once
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

#include <r2/systems/entity.h>

namespace r2 {
	class physics_system_state;

	class motion_state : public btMotionState {
		public:
			motion_state(scene_entity* entity);
			~motion_state();

			virtual void getWorldTransform(btTransform& worldTrans) const;
			virtual void setWorldTransform(const btTransform& worldTrans);

			scene_entity* entity;
	};

	class physics_component : public scene_entity_component {
		public:
			physics_component();
			~physics_component();

			virtual void destroy();

			void set_transform(const mat4f& transform);
			void set_shape(btCollisionShape* shape);
			void set_mass(f32 mass);

			inline btRigidBody* rigidBody() { return m_rigidBody; }

			bool creates_collision_events;
			bool update_mesh_when_moved;

		protected:
			friend class physics_sys;
			f32 m_mass;
			btCollisionShape* m_collisionShape;
			btRigidBody* m_rigidBody;
			motion_state* m_motionState;
			physics_system_state* m_sysState;
	};

	class physics_system_state : public engine_state_data {
		public:
			physics_system_state();
			~physics_system_state();

			btDefaultCollisionConfiguration* collisionConfig;
			btCollisionDispatcher* collisionDispatcher;
			btBroadphaseInterface* broadphaseInterface;
			btSequentialImpulseConstraintSolver* constraintSolver;
			btDiscreteDynamicsWorld* world;

			btAlignedObjectArray<btCollisionShape*> collisionShapes;
	};

	class physics_system_state_factory : public engine_state_data_factory {
		public:
			physics_system_state_factory();
			~physics_system_state_factory();

			virtual engine_state_data* create();
	};

	class physics_sys : public entity_system {
		public:
			~physics_sys();
			static physics_sys* get() {
				if (instance) return instance;
				instance = new physics_sys();
				return instance;
			}

			virtual const size_t component_size() const { return sizeof(physics_component); }

			virtual void initialize();
			virtual void initialize_entity(scene_entity* entity);
			virtual void deinitialize_entity(scene_entity* entity);
			virtual scene_entity_component* create_component(entityId id);
			virtual void bind(scene_entity_component* component, scene_entity* entity);
			virtual void unbind(scene_entity* entity);
			virtual void tick(f32 dt);
			virtual void handle(event* evt);

			engine_state_data_ref<physics_system_state>& physState() { return m_physState; }

			u32 max_simulation_steps_per_frame;
			f32 simulation_time_step;

		protected:
			physics_sys();
			static physics_sys* instance;
			engine_state_data_ref<physics_system_state> m_physState;
	};
};

