#include <r2/systems/physics_sys.h>
#include <r2/systems/cascade_functions.h>
#include <r2/engine.h>

#include <glm/gtc/type_ptr.hpp>

namespace r2 {
	motion_state::motion_state(scene_entity* _entity) : entity(_entity) {
	}

	motion_state::~motion_state() {
	}

	void motion_state::getWorldTransform(btTransform& worldTrans) const {
		mat4f transform = entity->transform->cascaded_property(&transform_component::transform, &cascade_mat4f);
		transform[0] = glm::normalize(transform[0]);
		transform[1] = glm::normalize(transform[1]);
		transform[2] = glm::normalize(transform[2]);
		worldTrans.setFromOpenGLMatrix(glm::value_ptr(transform));
	}

	void motion_state::setWorldTransform(const btTransform& worldTrans) {
		mat4f worldTransform;
		mat4f objectTransform;
		worldTrans.getOpenGLMatrix(glm::value_ptr(worldTransform));

		if (entity->parent() && entity->parent()->transform) {
			// worldTrans must be converted to the space of the parent
			mat4f ptransform = entity->parent()->transform->cascaded_property(&transform_component::transform, &cascade_mat4f);
			objectTransform = glm::inverse(ptransform) * worldTransform;
		} else objectTransform = worldTransform;

		entity->transform->transform = objectTransform;

		if (entity->mesh && entity->physics->update_mesh_when_moved) {
			render_node* node = entity->mesh->get_node();
			if (node->instances().is_valid()) {
				entity->mesh->set_instance_transform(worldTransform);
			} else {
				node->uniforms()->uniform_mat4f("transform", worldTransform);
			}
		}
	}



	physics_component::physics_component() {
		creates_collision_events = false;
		update_mesh_when_moved = true;
		m_mass = 0.0f;

		m_collisionShape = nullptr;
		m_rigidBody = nullptr;
		m_motionState = nullptr;
	}

	physics_component::~physics_component() {
	}

	void physics_component::destroy() {
		if (m_rigidBody) {
			m_sysState->world->removeCollisionObject(m_rigidBody);
			delete m_rigidBody;
			m_rigidBody = nullptr;
		}

		if (m_collisionShape) {
			i32 refcount = m_collisionShape->getUserIndex() - 1;
			m_collisionShape->setUserIndex(refcount);
			if (refcount == 0) {
				m_sysState->collisionShapes.remove(m_collisionShape);
				delete m_collisionShape;
			}
			m_collisionShape = nullptr;
		}

		if (m_motionState) delete m_motionState;
	}

	void physics_component::set_transform(const mat4f& transform) {
		if (!m_rigidBody) return;
		f32 sx = glm::length(vec3f(transform[0]));
		f32 sy = glm::length(vec3f(transform[1]));
		f32 sz = glm::length(vec3f(transform[2]));
		if (m_collisionShape) {
			m_collisionShape->setLocalScaling(btVector3(sx, sy, sz));
		}
		mat4f unscaled = transform;
		unscaled[0] = glm::normalize(unscaled[0]);
		unscaled[1] = glm::normalize(unscaled[1]);
		unscaled[2] = glm::normalize(unscaled[2]);

		btTransform t;
		t.setFromOpenGLMatrix(glm::value_ptr(unscaled));
		m_rigidBody->setWorldTransform(t);
	}

	void physics_component::set_shape(btCollisionShape* shape) {
		bool dynamic = m_mass != 0.0f;

		if (m_collisionShape) {
			i32 refcount = m_collisionShape->getUserIndex() - 1;
			m_collisionShape->setUserIndex(refcount);
			if (refcount == 0) m_sysState->collisionShapes.remove(m_collisionShape);

			if (shape) {
				refcount = shape->getUserIndex();
				if (refcount == -1) {
					refcount = 0;
					m_sysState->collisionShapes.push_back(shape);
				}
				refcount++;
				shape->setUserIndex(refcount);
			}

			m_collisionShape = shape;
		}

		if (shape) {
			mat4f transform = entity()->transform->cascaded_property(&transform_component::transform, &cascade_mat4f);
			f32 sx = glm::length(vec3f(transform[0]));
			f32 sy = glm::length(vec3f(transform[1]));
			f32 sz = glm::length(vec3f(transform[2]));
			shape->setLocalScaling(btVector3(sx, sy, sz));
		} else {
			if (m_rigidBody) {
				m_sysState->world->removeRigidBody(m_rigidBody);
				delete m_rigidBody;
				m_rigidBody = nullptr;
			}

			if (m_motionState) delete m_motionState;
			m_motionState = nullptr;

			return;
		}

		if (m_rigidBody) {
			btVector3 inertia(0.0f, 0.0f, 0.0f);
			if (dynamic && shape->getShapeType() != EMPTY_SHAPE_PROXYTYPE) shape->calculateLocalInertia(m_mass, inertia);
			m_rigidBody->setCollisionShape(shape);
			m_rigidBody->setMassProps(m_mass, inertia);
		} else {
			m_motionState = new motion_state(entity());
			
			btVector3 inertia(0.0f, 0.0f, 0.0f);
			if (dynamic && m_collisionShape->getShapeType() != EMPTY_SHAPE_PROXYTYPE) {
				m_collisionShape->calculateLocalInertia(m_mass, inertia);
			}

			btRigidBody::btRigidBodyConstructionInfo rbInfo(m_mass, m_motionState, m_collisionShape, inertia);
			m_rigidBody = new btRigidBody(rbInfo);

			m_sysState->world->addRigidBody(m_rigidBody);
		}
	}

	void physics_component::set_mass(f32 mass) {
		if (!m_collisionShape) m_mass = mass;
		else {
			btVector3 inertia;
			m_collisionShape->calculateLocalInertia(mass, inertia);
			m_rigidBody->setMassProps(mass, inertia);
		}
	}



	physics_system_state::physics_system_state() {
		collisionConfig = new btDefaultCollisionConfiguration();
		collisionDispatcher = new btCollisionDispatcher(collisionConfig);
		broadphaseInterface = new btDbvtBroadphase();
		constraintSolver = new btSequentialImpulseConstraintSolver();
		world = new btDiscreteDynamicsWorld(collisionDispatcher, broadphaseInterface, constraintSolver, collisionConfig);
	}

	physics_system_state::~physics_system_state() {
		btCollisionObjectArray& objs = world->getCollisionObjectArray();
		for (i32 i = world->getNumCollisionObjects() - 1; i >= 0; i--) {
			btCollisionObject* obj = objs[i];
			btRigidBody* body = btRigidBody::upcast(obj);
			world->removeCollisionObject(obj);
			delete obj;
		}

		/*
		for (i32 i = 0;i < collisionShapes.size();i++) {
			btCollisionShape* s = collisionShapes[i];
			collisionShapes[i] = nullptr;
			delete s;
		}
		collisionShapes.clear();
		*/

		delete world;
		delete constraintSolver;
		delete broadphaseInterface;
		delete collisionDispatcher;
		delete collisionConfig;
	}
	


	physics_system_state_factory::physics_system_state_factory() {
	}

	physics_system_state_factory::~physics_system_state_factory() {
	}

	engine_state_data* physics_system_state_factory::create() {
		return (engine_state_data*)new physics_system_state();
	}


	physics_sys* physics_sys::instance = nullptr;
	physics_sys::physics_sys() {
		max_simulation_steps_per_frame = 1;
		simulation_time_step = 1.0f / 60.0f;
	}

	physics_sys::~physics_sys() {
	}
	
	void physics_sys::initialize_entity(scene_entity* entity) {
		if (!entity->is_scripted()) return;
	}

	void physics_sys::deinitialize_entity(scene_entity* entity) {
		if (!entity->is_scripted()) return;
	}

	scene_entity_component* physics_sys::create_component(entityId id) {
		auto s = state();
		s.enable();
		auto out = s->create<physics_component>(id);
		m_physState.enable();
		out->m_sysState = m_physState.get();
		m_physState.disable();
		s.disable();
		return out;
	}

	void physics_sys::bind(scene_entity_component* component, scene_entity* entity) {
		using c = physics_component;
		entity->physics = component_ref<c*>(this, component->id());
	}

	void physics_sys::unbind(scene_entity* entity) {
		entity->physics.clear();
	}

	void physics_sys::initialize() {
		auto fac = new physics_system_state_factory();
		auto stateMgr = r2engine::get()->states();
		m_physState = stateMgr->register_state_data_factory<physics_system_state>(fac);
	}

	void physics_sys::tick(f32 dt) {
		m_physState.enable();
		m_physState->world->stepSimulation(dt, max_simulation_steps_per_frame, simulation_time_step);
		m_physState.disable();
	}

	void physics_sys::handle(event* evt) {
	}
};