#include <r2/engine.h>

int main(int argc,char** argv) {
    r2::r2engine* g_Engine = new r2::r2engine(argc,argv);
    const vector<string>& args = g_Engine->args();

    for(unsigned int i = 0;i < args.size();i++) {
        r2Log(g_Engine,"arg[%d]: %s",i,args[i].c_str());
    }

    int ret = g_Engine->run();
    delete g_Engine;
    return ret;
}
