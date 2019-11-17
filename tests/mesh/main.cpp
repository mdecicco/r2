#include <r2/engine.h>
#include <r2/managers/drivers/gl/driver.h>
using namespace r2;

#pragma pack(push, 1)
struct test_vertex {
    test_vertex(float _x = 0, float _y = 0, float _u = 0, float _v = 0) {
        x = _x; y = _y;
        u = _u; v = _v;
    }
    ~test_vertex() { }

	bool operator==(const test_vertex& rhs) {
		return x == rhs.x && y == rhs.y && u == rhs.u && v == rhs.v;
	}

    float x, y;
    float u, v;
};

struct test_instance_data {
    test_instance_data(float x = 0, float y = 0, float _scale = 1, i32 _colorIdx = 0) {
        pos_x = x; pos_y = y;
        scale = _scale;
		colorIdx = _colorIdx;
    }
    ~test_instance_data() { }

	bool operator==(const test_instance_data& rhs) {
		return pos_x == rhs.pos_x && pos_y == rhs.pos_y && scale == rhs.scale && colorIdx == rhs.colorIdx;
	}

    float pos_x, pos_y;
    float scale;
	i32 colorIdx;
};
#pragma pack(pop)

int main(int argc, char** argv) {
	r2::r2engine::create(argc, argv);
	r2::r2engine* eng = r2::r2engine::get();

	eng->open_window(200, 200, "mesh test", true);

	gl_render_driver* driver = new gl_render_driver(eng->renderer());
	eng->renderer()->set_driver(driver);

    scene* scene1 = eng->scenes()->create("scene1");

    // define mesh characteristics
    vertex_format vfmt;
    vfmt.add_attr(vat_vec2f); // pos
    vfmt.add_attr(vat_vec2f); // tex

    instance_format ifmt;
    ifmt.add_attr(iat_vec2f); // pos
	ifmt.add_attr(iat_float); // scale
	ifmt.add_attr(iat_int); // color index

    // create the mesh
	mesh_construction_data* mesh = new mesh_construction_data(&vfmt, it_unsigned_byte, &ifmt);
	mesh->set_max_vertex_count(4);
	mesh->set_max_index_count(6);
	mesh->set_max_instance_count(4);

    // single quad vertices (clockwise)
    mesh->append_vertex(test_vertex(-0.5, 0.5, 0.0, 1.0));
    mesh->append_vertex(test_vertex( 0.5, 0.5, 1.0, 1.0));
    mesh->append_vertex(test_vertex( 0.5,-0.5, 1.0, 0.0));
    mesh->append_vertex(test_vertex(-0.5,-0.5, 0.0, 0.0));

    // top right triangle indices
    mesh->append_index<u8>(0); // top left
    mesh->append_index<u8>(1); // top right
    mesh->append_index<u8>(2); // bottom right

    // bottom left triangle indices
    mesh->append_index<u8>(0); // top left
    mesh->append_index<u8>(2); // bottom right
    mesh->append_index<u8>(3); // bottom left

    // 4 instances, one in each corner
	mesh->append_instance(test_instance_data(-0.5, -0.5, 0.5, 0));
	mesh->append_instance(test_instance_data(-0.5,  0.5, 0.5, 1));
	mesh->append_instance(test_instance_data( 0.5, -0.5, 0.5, 2));
	mesh->append_instance(test_instance_data( 0.5,  0.5, 0.5, 3));

	// Test that the data copied to the mesh buffers properly
	{
		test_vertex vertices[4];
		memcpy(vertices, mesh->vertex_data(), mesh->vertexFormat().size() * 4);
		assert(vertices[0] == test_vertex(-0.5, 0.5, 0.0, 1.0));
		assert(vertices[1] == test_vertex( 0.5, 0.5, 1.0, 1.0));
		assert(vertices[2] == test_vertex( 0.5,-0.5, 1.0, 0.0));
		assert(vertices[3] == test_vertex(-0.5,-0.5, 0.0, 0.0));

		u8 indices[6];
		memcpy(indices, mesh->index_data(), mesh->indexType() * 6);
		assert(indices[0] == 0);
		assert(indices[1] == 1);
		assert(indices[2] == 2);
		assert(indices[3] == 0);
		assert(indices[4] == 2);
		assert(indices[5] == 3);

		test_instance_data instances[4];
		memcpy(instances, mesh->instance_data(), mesh->instanceFormat().size() * 4);
		assert(instances[0] == test_instance_data(-0.5, -0.5, 0.5, 0));
		assert(instances[1] == test_instance_data(-0.5,  0.5, 0.5, 1));
		assert(instances[2] == test_instance_data( 0.5, -0.5, 0.5, 2));
		assert(instances[3] == test_instance_data( 0.5,  0.5, 0.5, 3));
	}


    render_node* node = scene1->add_mesh(mesh);
	
	// Test that the data copied to the scene's buffer pool properly
	{
		test_vertex vertices[4];
		memcpy(vertices, node->vertices().buffer->data(), node->vertices().buffer->format().size() * 4);
		assert(vertices[0] == test_vertex(-0.5, 0.5, 0.0, 1.0));
		assert(vertices[1] == test_vertex( 0.5, 0.5, 1.0, 1.0));
		assert(vertices[2] == test_vertex( 0.5,-0.5, 1.0, 0.0));
		assert(vertices[3] == test_vertex(-0.5,-0.5, 0.0, 0.0));

		u8 indices[6];
		memcpy(indices, node->indices().buffer->data(), node->indices().buffer->type() * 6);
		assert(indices[0] == 0);
		assert(indices[1] == 1);
		assert(indices[2] == 2);
		assert(indices[3] == 0);
		assert(indices[4] == 2);
		assert(indices[5] == 3);

		test_instance_data instances[4];
		memcpy(instances, node->instances().buffer->data(), node->instances().buffer->format().size() * 4);
		assert(instances[0] == test_instance_data(-0.5, -0.5, 0.5, 0));
		assert(instances[1] == test_instance_data(-0.5,  0.5, 0.5, 1));
		assert(instances[2] == test_instance_data( 0.5, -0.5, 0.5, 2));
		assert(instances[3] == test_instance_data( 0.5,  0.5, 0.5, 3));
	}

	// load shader
	gl_shader_program* program = r2engine::get()->assets()->create<gl_shader_program>("mesh shader");
	program->load("./resource/test_shader.glsl");

	// create material
	uniform_format mfmt;
	mfmt.add_attr("color[0]", uat_vec3f);
	mfmt.add_attr("color[1]", uat_vec3f);
	mfmt.add_attr("color[2]", uat_vec3f);
	mfmt.add_attr("color[3]", uat_vec3f);
	node_material material("u_material", mfmt);
	material.set_shader(program);

	// create material instance for node
	node->material = material.instantiate(scene1);
	node->material->uniforms()->uniform_vec3f("color[0]", vec3f(1.0f, 1.0f, 1.0f));
	node->material->uniforms()->uniform_vec3f("color[1]", vec3f(1.0f, 0.0f, 0.0f));
	node->material->uniforms()->uniform_vec3f("color[2]", vec3f(0.0f, 1.0f, 0.0f));
	node->material->uniforms()->uniform_vec3f("color[3]", vec3f(0.0f, 0.0f, 1.0f));

	scene1->generate_vaos();
	scene1->sync_buffers();

    // start the engine
    int r = eng->run();

	delete node->material;
	eng->assets()->destroy(program);
    eng->scenes()->destroy(scene1);
	eng->shutdown();

    return r;
}
