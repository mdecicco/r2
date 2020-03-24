#include <r2/engine.h>
#include <r2/managers/drivers/gl/driver.h>
#include <r2/utilities/interpolator.hpp>
#include <glm/gtx/matrix_interpolation.hpp>
#include <glm/gtx/quaternion.hpp>
#include "swarmz.h"

using namespace r2;
#define SPAWN_RADIUS 120.0f
#define INSTANCE_COUNT 500
#define INSTANCE_UPDATE_FREQUENCY 60.0f

mvector<mstring> split(const mstring& str, const mstring& sep) {
	char* cstr = const_cast<char*>(str.c_str());
	char* current;
	mvector<mstring> arr;
	current = strtok(cstr, sep.c_str());
	while (current!=NULL) {
		arr.push_back(current);
		current = strtok(NULL, sep.c_str());
	}
	return arr;
}

render_node* load_obj(const mstring& obj, scene* destScene) {
	auto file = r2engine::get()->files()->load(obj, DM_TEXT, "obj");
	if (file) {
		mvector<mstring> lines;
		while(!file->at_end(1)) {
			mstring line;
			if(!file->read_line(line)) {
				r2Error("Failed to read %s\n", obj.c_str());
				r2engine::get()->files()->destroy(file);
				return nullptr;
			}
			lines.push_back(line);
		}
		r2engine::get()->files()->destroy(file);

		shader_program* shader = destScene->load_shader("./resource/unscripted_test/shader.glsl", "shader");
		if (!shader) {
			r2Error("Failed to load shader\n");
			return nullptr;
		}

		struct vertex {
			vec3f position;
			vec3f normal;
		};
		vector<vertex> vertices;
		vector<u32> indices;
		vector<vec3f> positions;
		vector<vec3f> normals;
		vector<vec2f> texcoords;
		unordered_map<mstring, u32> fvIndices;
		for(auto line : lines) {
			mvector<mstring> comps = split(line, " ");
			if (comps.size() > 0) {
				if (comps[0] == "v") {
					positions.push_back(vec3f(
						atof(comps[1].c_str()),
						atof(comps[2].c_str()),
						atof(comps[3].c_str())
					));
				} else if (comps[0] == "vn") {
					normals.push_back(vec3f(
						atof(comps[1].c_str()),
						atof(comps[2].c_str()),
						atof(comps[3].c_str())
					));
				} else if (comps[0] == "vt") {
					texcoords.push_back(vec2f(
						atof(comps[1].c_str()),
						atof(comps[2].c_str())
					));
				} else if (comps[0] == "f") {
					if (comps.size() == 4) {
						for(u8 i = 1;i < comps.size();i++) {
							if (fvIndices.find(comps[i]) != fvIndices.end()) indices.push_back(fvIndices[comps[i]]);
							else {
								vector<i32> idxGroup;
								mstring idx = "";
								for(char ch : comps[i]) {
									if (ch == '/') {
										if (idx.length() > 0) idxGroup.push_back(atoi(idx.c_str()));
										else idxGroup.push_back(-1);
										idx = "";
									} else idx += ch;
								}

								if (idx.length() > 0) idxGroup.push_back(atoi(idx.c_str()));
								else idxGroup.push_back(-1);

								vertex v;
								for(i32 j = 0;j < idxGroup.size();j++) {
									i32 gidx = idxGroup[j];
									if (j == 0 && gidx != -1) v.position = positions[gidx - 1];
									//if (j == 1 && gidx != -1) v.texcoord = texcoords[gidx - 1];
									if (j == 2 && gidx != -1) v.normal = normals[gidx - 1];
								}

								vertices.push_back(v);
								indices.push_back(vertices.size() - 1);
								fvIndices[comps[i]] = vertices.size() - 1;
							}
						}
					}
				}
			}
		}

		vertex_format* vfmt = new vertex_format();
		vfmt->add_attr(vat_vec3f);
		vfmt->add_attr(vat_vec3f);

		instance_format* ifmt = new instance_format();
		ifmt->add_attr(iat_mat4f);

		uniform_format* mfmt = new uniform_format();
		mfmt->add_attr("color", uat_vec3f);

		mesh_construction_data* mesh = new mesh_construction_data(vfmt, it_unsigned_short, ifmt);
		mesh->set_max_index_count(indices.size());
		mesh->set_max_vertex_count(vertices.size());
		mesh->set_max_instance_count(INSTANCE_COUNT);
		for(vertex v : vertices) mesh->append_vertex(v);
		for(i32 idx : indices) mesh->append_index<u16>(idx);
		for(u32 i = 0;i < INSTANCE_COUNT;i++) mesh->append_instance(mat4f(1));

		render_node* node = destScene->add_mesh(mesh);
		node_material* mtrl = new node_material("u_material", mfmt);
		mtrl->set_shader(shader);
		node->set_material_instance(mtrl->instantiate(destScene));
		node->material_instance()->uniforms()->uniform_vec3f("color", vec3f(0.75f, 0.1f, 0.2f));

		return node;
	}
}

f32 random(f32 min = -1.0f, f32 max = 1.0f) {
	return min + f32(rand()) / (f32(RAND_MAX) / (max - min));
}

class motion_component : public scene_entity_component {
	public:
		motion_component(entityId id) : position(vec3f(0, 0, 0), 30.0f, interpolate::easeInOutCubic) {
			time = id * 3.0f;
			rotAxis = glm::normalize(vec3f(random(), random(), random()));
			position = vec3f(random(-SPAWN_RADIUS, SPAWN_RADIUS), random(-SPAWN_RADIUS, SPAWN_RADIUS), random(-SPAWN_RADIUS, SPAWN_RADIUS));
		}

		f32 time;
		interpolator<vec3f> position;
		vec3f rotAxis;
		mat4f transform;
};

class motion_system : public entity_system, periodic_update {
	public:
		motion_system() { }
		~motion_system() {
			destroy_periodic_update();
			delete swarm;
		}
		static motion_system* get() {
			if (instance) return instance;
			instance = new motion_system();
			return instance;
		}
		virtual const size_t component_size() const { return sizeof(motion_component); }

		virtual void initialize() {
			initialize_periodic_update();
			start_periodic_updates();
			setUpdateFrequency(INSTANCE_UPDATE_FREQUENCY);

			for(u32 i = 0;i < INSTANCE_COUNT;i++) {
				vec3f pos(random(), random(), random());
				pos = glm::normalize(pos) * SPAWN_RADIUS * 0.5f;
				boids.push_back(sw::Boid(sw::Vec3(pos.x, pos.y, pos.z), sw::Vec3(random(), random(), random())));
			}
			swarm = new sw::Swarm(&boids);
			swarm->PerceptionRadius = 10.0f;
			swarm->MaxAcceleration = 30.0f;
			swarm->SeparationWeight = 122.3f;
			swarm->CohesionWeight = 1.0f;
		}
		virtual void initialize_entity(scene_entity* entity) {
		}
		virtual void deinitialize_entity(scene_entity* entity) {
		}
		virtual scene_entity_component* create_component(entityId id) {
			auto s = state();
			s.enable();
			auto out = s->create<motion_component>(id, id);
			s.disable();
			return out;
		}
		virtual void bind(scene_entity_component* component, scene_entity* entity) {
		}
		virtual void unbind(scene_entity* entity) {
		}
		virtual void tick(f32 dt) {
			update(dt);
		}
		virtual void handle(event* evt) {
		}
		virtual void doUpdate(f32 frameDelta, f32 updateDelta) {
			if (r2engine::input()->joystick_count() > 0) {
				f32 rtrig = f32(r2engine::input()->joystick(0)->getJoyStickState().mAxes[5].abs) / 32767.0f;
				updateDelta *= rtrig;
			}
			swarm->Update(updateDelta);
			for (auto& boid : boids) {
				vec3f pos(boid.Position.X, boid.Position.Y, boid.Position.Z);
				f32 edgeDelta = glm::length(pos) - (SPAWN_RADIUS * 0.5f);
				vec3f forceDir = glm::normalize(-pos);
				boid.Velocity.X += forceDir.x * (edgeDelta * 0.6f) * updateDelta;
				boid.Velocity.Y += forceDir.y * (edgeDelta * 0.6f) * updateDelta;
				boid.Velocity.Z += forceDir.z * (edgeDelta * 0.6f) * updateDelta;
				boid.Position += boid.Velocity * updateDelta;
			}

			auto s = state();
			s.enable();
			u32 idx = 0;
			s->for_each<motion_component>([updateDelta, this, &idx](motion_component* component) {
				component->time += updateDelta;
				f32 jiggleAccel = sin(component->time * 6.0f);
				f32 fac = fmod(component->time, 6.28318530718f);
				f32 jiggleFactor = sin(fac) * jiggleAccel;
				vec3f scale = vec3f(
					1 + (jiggleFactor * 0.2),
					1 + (jiggleFactor * 0.8),
					1 + (jiggleFactor * 0.2)
				);

				sw::Boid& b = this->boids[idx];

				vec3f pos = vec3f(b.Position.X, b.Position.Y, b.Position.Z);
				vec3f vel = vec3f(b.Velocity.X, b.Velocity.Y, b.Velocity.Z);
				glm::quat rot = glm::toQuat(glm::transpose(glm::lookAt(pos, pos + vel, vec3f(0, 1, 0))));
				rot = glm::rotate(rot, vec3f(0, glm::radians(-90.0f), 0));

				mat4f transform(1.0f);
				transform = glm::translate(transform, pos);//(vec3f)component->position);
				if (idx % 3 == 0) transform = glm::rotate(transform, glm::radians(component->time * 50), component->rotAxis);
				else transform = glm::rotate(transform, glm::angle(rot), glm::axis(rot));
				transform = glm::scale(transform, scale * 0.8f);
				component->transform = transform;


				//if (i32(time) % 30 == 0) {
					//component->position = vec3f(random(-SPAWN_RADIUS, SPAWN_RADIUS), random(-SPAWN_RADIUS, SPAWN_RADIUS), random(-SPAWN_RADIUS, SPAWN_RADIUS));
				//}

				scene_entity* entity = component->entity();
				mat4f current = component->transform;
				entity->transform->transform = current;
				entity->mesh->set_instance_data(current);

				idx++;

				// don't break
				return true;
			});
			s.disable();
		}

	protected:
		static motion_system* instance;
		vector<sw::Boid> boids;
		sw::Swarm* swarm;
};
motion_system* motion_system::instance = nullptr;

class camera_entity : public scene_entity {
	public:
		camera_entity() : scene_entity("TestCamera") {
			// Anything allocated or deallocated within this class
			// will be inside of the memory scope of the state it
			// resides in (or the global scope if it was created 
			// outside of any state). If this entity was allocated
			// outside of any state, it will become inactive once
			// a state becomes active.

			time = 0.0f;
			deadzone = 0.2f;
		}

		~camera_entity() {
		}

		virtual void onInitialize() {
			setUpdateFrequency(60.0f);
			transform_sys::get()->addComponentTo(this);
			camera_sys::get()->addComponentTo(this);
			camera->activate();
			transform->transform = glm::lookAt(vec3f(0, SPAWN_RADIUS + 20.0f, -(SPAWN_RADIUS + 20.0f)), vec3f(0, 0, 0), vec3f(0, 1, 0));
			j_pos = vec3f(0, 0, 0);
			j_rot = mat4f(1.0f);
			if (r2engine::input()->joystick_count() > 0) {
				transform->transform = glm::translate(j_rot, j_pos);
			}


			vec2f screen = r2engine::get()->window()->get_size();
			camera->projection = glm::perspective(glm::radians(60.0f), screen.y / screen.x, 0.01f, 500.0f);
		}

		virtual void onUpdate(f32 frameDt, f32 updateDt) {
			time += updateDt;

			if (r2engine::input()->joystick_count() > 0) {
				auto js = r2engine::input()->joystick(0);
				auto jState = js->getJoyStickState();
				lstick = vec2f(f32(jState.mAxes[1].abs) / 32767.0f, f32(jState.mAxes[0].abs) / 32767.0f);
				rstick = vec2f(f32(jState.mAxes[3].abs) / 32767.0f, f32(jState.mAxes[2].abs) / 32767.0f);

				bool changed = false;

				if (glm::length(lstick) > deadzone) {
					vec3f translate = vec4f(-lstick.x, 0, -lstick.y, 1.0f) * j_rot;
					translate *= 25.0f * updateDt;
					j_pos += translate;
					changed = true;
				}

				if (glm::length(rstick) > deadzone) {
					if (rstick.x > deadzone || rstick.x < -deadzone) {
						vec3f axis = vec4f(0, 1, 0, 1);
						j_rot *= glm::rotate(mat4f(1.0f), rstick.x * 2.0f * updateDt, axis);
					}
					if (rstick.y > deadzone || rstick.y < -deadzone) {
						vec3f axis = vec4f(1, 0, 0, 1) * j_rot;
						j_rot *= glm::rotate(mat4f(1.0f), rstick.y * 2.0f * updateDt, axis);
					}
					changed = true;
				}

				if (changed) transform->transform = glm::translate(j_rot, j_pos);
			} else {
				mat4f r(1.0f);
				vec4f position = vec4f(0.0f, SPAWN_RADIUS + 20.0f, -(SPAWN_RADIUS + 20.0f), 1.0f) * glm::rotate(r, glm::radians(time), vec3f(0, 1, 0));
				transform->transform = glm::lookAt(vec3f(position), vec3f(0, 0, 0), vec3f(0, 1, 0));
			}
		}

		virtual void onEvent(event* evt) {
		}

		virtual void willBeDestroyed() {
		}

		virtual void belowFrequencyWarning(f32 percentLessThanDesired, f32 desiredFreq, f32 timeSpentLowerThanDesired) {
			// This is called when the entity has been updating
			// at a frequency that is lower than the frequency
			// specified with setUpdateFrequency for longer than
			// internally specified duration of time. If the
			// entity continues to update below the specified
			// frequency, this function will be called at
			// intervals of time specified internally.
		}

		f32 time;
		vec2f lstick;
		vec2f rstick;
		f32 deadzone;

		vec3f j_pos;
		mat4f j_rot;
};

class test_entity : public scene_entity {
	public:
		test_entity(render_node* _mesh) : scene_entity("TestEntity"), node(_mesh), position(vec3f(0, 0, 0), 30.0f, interpolate::easeInOutCubic) {
			// Anything allocated or deallocated within this class
			// will be inside of the memory scope of the state it
			// resides in (or the global scope if it was created 
			// outside of any state). If this entity was allocated
			// outside of any state, it will become inactive once
			// a state becomes active.
		}

		~test_entity() {
		}

		virtual void onInitialize() {
			setUpdateFrequency(60.0f);
			stop_periodic_updates();
			transform_sys::get()->addComponentTo(this);
			if (id() != 2) motion_system::get()->addComponentTo(this);
			mesh_sys::get()->addComponentTo(this);
			mesh->set_node(node);
			r2engine::get()->remove_child(this);
		}

		virtual void onUpdate(f32 frameDt, f32 updateDt) {
			time += updateDt;
			//printf("TestEntity::onUpdate(%0.2f ms, %0.2f ms)\n", frameDt * 1000.0f, updateDt * 1000.0f);
			f32 jiggleAccel = sin(time * 6.0f);
			f32 fac = fmod(time, 6.28318530718f);
			f32 jiggleFactor = sin(fac) * jiggleAccel;
			vec3f scale = vec3f(
				1 + (jiggleFactor * 0.2),
				1 + (jiggleFactor * 0.8),
				1 + (jiggleFactor * 0.2)
			);

			mat4f t(1.0f);
			t = glm::identity<mat4f>();
			t = glm::translate(t, (vec3f)position);
			t = glm::rotate(t, glm::radians(time * 50), rotAxis);
			t = glm::scale(t, scale * 0.8f);

			if (i32(time) % 30 == 0) {
				position = vec3f(random(-SPAWN_RADIUS, SPAWN_RADIUS), random(-SPAWN_RADIUS, SPAWN_RADIUS), random(-SPAWN_RADIUS, SPAWN_RADIUS));
			}

			transform->transform = t;
			mesh->set_instance_data(t);
		}

		virtual void onEvent(event* evt) {
		}

		virtual void willBeDestroyed() {
		}

		virtual void belowFrequencyWarning(f32 percentLessThanDesired, f32 desiredFreq, f32 timeSpentLowerThanDesired) {
			// This is called when the entity has been updating
			// at a frequency that is lower than the frequency
			// specified with setUpdateFrequency for longer than
			// internally specified duration of time. If the
			// entity continues to update below the specified
			// frequency, this function will be called at
			// intervals of time specified internally.
		}

		render_node* node;
		f32 time;
		interpolator<vec3f> position;
		vec3f rotAxis;
};

class test_state : public state {
	public:
		test_state() : state("TestState", MBtoB(20)), cubes(nullptr) {
			// This state's memory has not been allocated yet. Any
			// allocations made here will be either in the global
			// scope, or the scope of the currently active state.
			// Be careful not to allocate here, or to explicitly
			// allocate in the global scope via:
			// memory_man::push_current(memory_man::global())
			// ...allocate stuff...
			// memory_man::pop_current()
		}

		~test_state() {
			// This state's memory should already be deallocated
			// by this point, unless the user deleted the state
			// while it was active (don't do that)
		}

		virtual void onInitialize() {
			// The state's memory has been allocated, and is
			// currently active. From here until becameInactive
			// is called, all allocations made in this class
			// will be in the state's own memory scope (unless
			// explicitly allocated in the global scope using
			// memory_man::[push/pop]_current)
			setUpdateFrequency(1.0f);
			camera = new camera_entity();
			mesh = load_obj("./resource/unscripted_test/teapot.obj", getScene());
			cubes = new dynamic_pod_array<test_entity*>();
			for(u32 i = 0;i < INSTANCE_COUNT;i++) {
				cubes->push(new test_entity(mesh));
			}
			glEnable(GL_MULTISAMPLE);
		}

		virtual void willBecomeActive() {
			// The state is going to become active. The previous
			// state is still active at this point, but will be
			// deactivated immediately after this function returns.
			// This state will be activated immediately after that
		}

		virtual void becameActive() {
			// The state was activated completely, and the previous
			// state (if any) has been cleared
		}

		virtual void willBecomeInactive() {
			// Becoming inactive means, among other things, that
			// the entire block of memory used by this state will
			// either be cleared or temporarily deallocated.
			// You don't _have_ to destroy entities here, or
			// deallocate anything allocated within this state's
			// memory, but you should...
			cubes->for_each([](test_entity** entity) {
				(*entity)->destroy();
				(*entity) = nullptr;
				return true;
			});
			delete cubes;
			cubes = nullptr;
		}

		virtual void becameInactive() {
			// Anything allocated here will be deallocated immediately
			// after this function returns, unless it's explicitly
			// allocated in the global scope via:
			// memory_man::push_current(memory_man::global())
			// ...allocate stuff...
			// memory_man::pop_current()
		}

		virtual void willBeDestroyed() {
			// By the time this function is called, unless the state is
			// currently active (should never happen unless the user
			// deletes the state for some reason), the state's memory
			// will already be deallocated, and any allocations made here
			// will be made either in the global scope, or the currently
			// active state's scope. Be careful to not allocate here, or
			// to specify which scope the allocation _should_ be made in.
			// This includes allocations made internally by std containers
		}

		virtual void onUpdate(f32 frameDt, f32 updateDt) {
			// Will be called as frequently as the user specifies with
			// setUpdateFrequency(frequency in Hz), as long as the
			// application is able to achieve that frequency.
			// Rendering probably can't be done here, as it's called
			// outside of the context of the frame.

			printf("TestState::onUpdate(%.2f ms, %.2f ms)\n", frameDt * 1000.0f, updateDt * 1000.0f);
		}

		virtual void onRender() {
			// Will be called once per frame
			// ImGui::Text("Memory: %s / %s", format_size(getUsedMemorySize()), format_size(getMaxMemorySize()));
			ImGui::Text("FPS: %.2f", r2engine::get()->fps());
			if (ImGui::Button("Reset State", ImVec2(190, 20))) {
				r2engine::get()->activate_state("TestState");
			}
		}

		virtual void onEvent(event* evt) {
			// Will be called whenever a non-internal event is fired,
			// or in the case of deferred events, at the beginning of
			// the engine tick (before onUpdate would be called)
		}

		camera_entity* camera;
		dynamic_pod_array<test_entity*>* cubes;
		render_node* mesh;
};

int main(int argc, char** argv) {
	memory_man::current()->slow_check();
	r2engine::register_system(motion_system::get());
	memory_man::current()->slow_check();
	r2engine::create(argc, argv);
	memory_man::current()->slow_check();
	auto eng = r2engine::get();
	memory_man::current()->slow_check();

	eng->open_window(512, 512, "Unscripted Test", true);
	memory_man::current()->slow_check();
	eng->renderer()->set_driver(new gl_render_driver(eng->renderer()));
	memory_man::current()->slow_check();

	test_state* state = new test_state();
	eng->states()->register_state(state);
	eng->states()->activate("TestState");

	int ret = eng->run();
	eng->shutdown();
	return ret;
}
