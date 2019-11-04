#include <r2/engine.h>

int main(int argc, char** argv) {
	r2::r2engine::create(argc, argv);
    r2::r2engine* g_Engine = r2::r2engine::get();
    const vector<string>& args = g_Engine->args();

    for(unsigned int i = 0;i < args.size();i++) {
        r2Log(g_Engine,"arg[%d]: %s",i,args[i].c_str());
    }

    int ret = g_Engine->run();

	g_Engine->shutdown();
    return ret;
}