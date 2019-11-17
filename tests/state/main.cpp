#include <r2/engine.h>

int main(int argc, char** argv) {
	r2::r2engine::create(argc, argv);
    r2::r2engine* g_Engine = r2::r2engine::get();
    const vector<string>& args = g_Engine->args();

    for(unsigned int i = 0;i < args.size();i++) {
        r2Log("arg[%d]: %s", i, args[i].c_str());
    }

	//r2::state* s0 = new r2::state(g_Engine, ".\\resource\\state.js");
	//r2::state* s1 = new r2::state(g_Engine, ".\\resource\\otherstate.js");

	//g_Engine->states()->register_state(s0);
	//g_Engine->states()->register_state(s1);

	//g_Engine->states()->activate("TestState");
	//g_Engine->states()->activate("OtherTestState");

	//s1->setUpdateFrequency(20);

	g_Engine->scripts()->executeFile("./resource/init.js");

	int ret = g_Engine->run();

	//g_Engine->states()->unregister_state(s0);
	//g_Engine->states()->unregister_state(s1);

	//delete s1;
	//delete s0;

	g_Engine->shutdown();
    return ret;
}
