#include <r2/engine.h>

int main(int argc, char** argv) {
	r2::r2engine::create(argc, argv);
    r2::r2engine* eng = r2::r2engine::get();

	eng->scripts()->executeFile("./resource/state_test/init.js");

	int ret = eng->run();

	eng->shutdown();
    return ret;
}
