#include <r2/engine.h>

int main(int argc, char** argv) {
	r2::r2engine::create(argc, argv);
    r2::r2engine* eng = r2::r2engine::get();
    const r2::mvector<r2::mstring>& args = eng->args();

    for(r2::u8 i = 0;i < args.size();i++) {
        r2Log("arg[%d]: %s", i, args[i].c_str());
    }

    int ret = eng->run();

	eng->shutdown();
    return ret;
}