#include <r2/engine.h>
using namespace r2;

int main(int argc, char** argv) {
	r2engine::create(argc, argv);
	auto eng = r2engine::get();
	auto args = eng->args();

	for(u32 i = 0; i < args.size(); i++) r2Log("arg[%d]: %s", i, args[i].c_str());

	int ret = eng->run();
	eng->shutdown();
	return ret;
}
