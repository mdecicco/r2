#include <r2/engine.h>

#include <r2/managers/drivers/gl/driver.h>
using namespace r2;

int main(int argc, char** argv) {
	r2::r2engine::create(argc, argv);
	r2::r2engine* eng = r2::r2engine::get();

	eng->open_window(200, 200, "uniform test", true);

	gl_render_driver* driver = new gl_render_driver(eng->renderer());
	eng->renderer()->set_driver(driver);

	// vec2* should be left alone
	{
		vec2f in(1.0f, 2.0f);
		vec2f out;
		driver->serialize_uniform_value(&in, &out, uat_vec2f);
		assert(in == out);
	}

	// vec3* should be cast to a vec4*
	{
		vec3f in(1.0f, 2.0f, 3.0f);
		vec4f out;
		driver->serialize_uniform_value(&in, &out, uat_vec3f);
		assert(in == vec3f(out));
		assert(out.w == 0.0f);
	}

	// mat2* should be cast to mat2x4*
	{
		mat2f in(1.0f, 2.0f, 3.0f, 4.0f);
		vec4f out[2];
		driver->serialize_uniform_value(&in, &out, uat_mat2f);
		assert(in[0] == vec2f(out[0]));
		assert(out[0].z == 0.0f);
		assert(out[0].w == 0.0f);
		assert(in[1] == vec2f(out[1]));
		assert(out[1].z == 0.0f);
		assert(out[1].w == 0.0f);
	}

	// mat3* should be cast to mat3x4*
	{
		mat3f in(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f);
		vec4f out[3];
		driver->serialize_uniform_value(&in, &out, uat_mat3f);
		assert(in[0] == vec3f(out[0]));
		assert(out[0].w == 0.0f);
		assert(in[1] == vec3f(out[1]));
		assert(out[1].w == 0.0f);
		assert(in[2] == vec3f(out[2]));
		assert(out[2].w == 0.0f);
	}

	// mat4* should be left alone
	{
		mat4f in(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f);
		vec4f out[4];
		driver->serialize_uniform_value(&in, &out, uat_mat4f);
		assert(in[0] == vec4f(out[0]));
		assert(in[1] == vec4f(out[1]));
		assert(in[2] == vec4f(out[2]));
		assert(in[3] == vec4f(out[3]));
	}

	eng->shutdown();
	return 0;
}
