#include <r2/engine.h>

int main(int argc,char** argv) {
    r2::r2engine* eng = new r2::r2engine(argc,argv);
    r2::scene_man* scenes = eng->scenes();

    r2::scene* scene1 = scenes->create("scene1"); // should pass
    r2::scene* scene2 = scenes->create("scene1"); // should fail
    r2::scene* scene3 = scenes->create("scene2"); // should pass

    int r = eng->run();
    scenes->destroy(scene1); // should pass
    scenes->destroy(scene2); // should cause warning

    scene3->name(); // prevents annoying unused variable warning

    delete eng; // scene manager should warn about undestroyed scene (scene2)
    return r;
}
