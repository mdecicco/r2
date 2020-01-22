#include <r2/engine.h>
#include <r2/managers/drivers/gl/driver.h>
#include <r2/utilities/interpolator.hpp>
#include <glm/gtx/matrix_interpolation.hpp>
#include <glm/gtx/quaternion.hpp>
#include <r2/utilities/debug_drawer.h>
#include <r2/utilities/fly_camera.h>
#include <r2/utilities/utils.h>

#include <al.h>
#include <alc.h>

#define INSTANCE_COUNT 10

using namespace r2;
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

		shader_program* shader = destScene->load_shader("./resource/physics_test/shader.glsl", "shader");
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
		ifmt->add_attr(iat_mat4f, true);

		uniform_format* mfmt = new uniform_format();
		mfmt->add_attr("color", uat_vec3f);

		mesh_construction_data* mesh = new mesh_construction_data(vfmt, it_unsigned_short, ifmt);
		mesh->set_max_index_count(indices.size());
		mesh->set_max_vertex_count(vertices.size());
		mesh->set_max_instance_count(INSTANCE_COUNT + 1);
		for(vertex v : vertices) mesh->append_vertex(v);
		for(i32 idx : indices) mesh->append_index<u16>(idx);
		for(u32 i = 0;i < INSTANCE_COUNT + 1;i++) mesh->append_instance(mat4f(1));

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

class physics_drawer : public btIDebugDraw {
	public:
		physics_drawer(debug_drawer* _drawer) : drawer(_drawer) {
			debugMode = btIDebugDraw::DBG_DrawAabb | btIDebugDraw::DBG_DrawWireframe;
		}
		
		~physics_drawer() { }

		virtual void reportErrorWarning(const char* warningString) {
			r2Warn(warningString);
		}

		virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) {
			drawer->line(vec3f(from.x(), from.y(), from.z()), vec3f(to.x(), to.y(), to.z()), vec4f(color.x(), color.y(), color.z(), 1.0f));
		}

		virtual void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) {
		}

		virtual void draw3dText(const btVector3& location, const char* textString) {
		}

		virtual void setDebugMode(int mode) {
			debugMode = mode;
		}

		virtual int getDebugMode() const {
			return debugMode;
		}

		i32 debugMode;
		debug_drawer* drawer;
};

class test_entity : public scene_entity {
	public:
		test_entity(render_node* _mesh, bool _is_floor = false) : scene_entity("TestEntity"), node(_mesh), is_floor(_is_floor) {
		}

		~test_entity() {
		}

		virtual void onInitialize() {
			setUpdateFrequency(60.0f);
			stop_periodic_updates();
			transform_sys::get()->addComponentTo(this);
			mesh_sys::get()->addComponentTo(this);
			mesh->set_node(node);
			physics_sys::get()->addComponentTo(this);
			if (is_floor) {
				mat4f t(1.0f);
				t = glm::translate(t, vec3f(0, -1.0f, 0));
				t = glm::scale(t, vec3f(20, 0.25f, 20));
				transform->transform = t;
				mesh->set_instance_transform(t);
			} else {
				mat4f t(1.0f);
				t = glm::translate(t, vec3f(0, 10.0f, 0));
				transform->transform = t;
				mesh->set_instance_transform(t);
			}
			physics->set_mass(is_floor ? 0 : 10);
			physics->set_shape(new btBoxShape(btVector3(0.5f, 0.5f, 0.5f)));
			r2engine::get()->remove_child(this);
		}

		virtual void onUpdate(f32 frameDt, f32 updateDt) {
		}

		virtual void onEvent(event* evt) {
		}

		virtual void willBeDestroyed() {
		}

		virtual void belowFrequencyWarning(f32 percentLessThanDesired, f32 desiredFreq, f32 timeSpentLowerThanDesired) {
		}

		render_node* node;
		bool is_floor;
};

class test_state : public state {
	public:
		test_state() : state("TestState", MBtoB(15)), cubes(nullptr), draw(nullptr) {
		}

		~test_state() {
		}

		virtual void onInitialize() {
			setUpdateFrequency(0.25f);
			camera = new fly_camera_entity();
			cubes = new dynamic_pod_array<test_entity*>();

			mesh = load_obj("./resource/physics_test/cube.obj", getScene());

			cubes->push(new test_entity(mesh, true));

			for(u32 i = 0;i < INSTANCE_COUNT;i++) {
				cubes->push(new test_entity(mesh));
			}

			glEnable(GL_MULTISAMPLE);

			draw_shader = getScene()->load_shader("./resource/physics_test/debug.glsl", "debug_shader");
			draw = new debug_drawer(getScene(), draw_shader, 8192 * 2, 8192 * 3);
			physDrawer = new physics_drawer(draw);

			auto& ps = physics_sys::get()->physState();
			ps.enable();
			ps->world->setDebugDrawer(physDrawer);
			ps.disable();


		}

		virtual void willBecomeActive() {
		}

		virtual void becameActive() {
		}

		virtual void willBecomeInactive() {
			cubes->for_each([](test_entity** entity) {
				(*entity)->destroy();
				(*entity) = nullptr;
				return true;
			});
			delete cubes;
			cubes = nullptr;
			delete draw;
			delete physDrawer;
		}

		virtual void becameInactive() {
		}

		virtual void willBeDestroyed() {
		}

		virtual void onUpdate(f32 frameDt, f32 updateDt) {
			printf("TestState::onUpdate(%.2f ms, %.2f ms)\n", frameDt * 1000.0f, updateDt * 1000.0f);
			test_entity* e = *cubes->at(1 + (u32)random(0, INSTANCE_COUNT - 1));
			mat4f transform = glm::translate(mat4f(1.0f), vec3f(0.0f, 5.0f, 0.0f));
			e->physics->set_transform(transform);
			e->physics->rigidBody()->activate(true);
			e->physics->rigidBody()->setLinearVelocity(btVector3(0.0f, 0.0f, 0.0f));
			e->physics->rigidBody()->applyTorqueImpulse(btVector3(random() * 15.0f, random() * 15.0f, random() * 15.0f));
		}

		virtual void onRender() {
			draw->begin();
			draw->line(vec3f(0.0f, 0.0f, 0.0f), vec3f(1.0f, 0.0f, 0.0f), vec4f(1.0f, 0.0f, 0.0f, 1.0f));
			draw->line(vec3f(0.0f, 0.0f, 0.0f), vec3f(0.0f, 1.0f, 0.0f), vec4f(0.0f, 1.0f, 0.0f, 1.0f));
			draw->line(vec3f(0.0f, 0.0f, 0.0f), vec3f(0.0f, 0.0f, 1.0f), vec4f(0.0f, 0.0f, 1.0f, 1.0f));


			auto& ps = physics_sys::get()->physState();
			ps.enable();
			ps->world->debugDrawWorld();
			ps.disable();
			draw->end();

			ImGui::Text("FPS: %.2f", r2engine::get()->fps());
			if (ImGui::Button("Reset State", ImVec2(190, 20))) {
				r2engine::get()->activate_state("TestState");
			}
		}

		virtual void onEvent(event* evt) {
		}

		fly_camera_entity* camera;
		dynamic_pod_array<test_entity*>* cubes;
		render_node* mesh;
		shader_program* draw_shader;
		debug_drawer* draw;
		physics_drawer* physDrawer;
};

int main(int argc, char** argv) {
	r2engine::create(argc, argv);
	auto eng = r2engine::get();

	eng->open_window(512, 512, "Physics Test", true);
	eng->renderer()->set_driver(new gl_render_driver(eng->renderer()));

	test_state* state = new test_state();
	eng->states()->register_state(state);
	eng->states()->activate("TestState");

	int ret = eng->run();
	eng->shutdown();
	return ret;
}
