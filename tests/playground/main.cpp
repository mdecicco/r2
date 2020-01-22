#include <r2/engine.h>
#include <r2/managers/drivers/gl/driver.h>
using namespace r2;

int main(int argc, char** argv) {
	r2engine::create(argc, argv);
	auto eng = r2engine::get();
	eng->open_window(512, 512, "", true);
	eng->renderer()->set_driver(new gl_render_driver(eng->renderer()));

	int ret = eng->run();
	eng->shutdown();
	return ret;
}
