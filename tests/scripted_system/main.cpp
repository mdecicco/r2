#include <r2/engine.h>
using namespace r2;

int main(int argc, char** argv) {
	r2engine::create(argc, argv);
	r2engine::get()->scripts()->executeFile("./resource/system_test/test.js");

	int ret = r2engine::get()->run();

	r2engine::shutdown();
	return ret;
}
