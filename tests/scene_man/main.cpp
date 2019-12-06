#include <r2/engine.h>
#include <r2/managers/drivers/gl/driver.h>

int main(int argc,char** argv) {
	r2::r2engine::create(argc, argv);
	r2::r2engine* eng = r2::r2engine::get();
    r2::scene_man* scenes = eng->scenes();

	// Can't create a scene before the render driver has been set
	r2::scene* scene0 = scenes->create("scene0");
	assert(scene0 == nullptr);

	eng->open_window(100, 100, "ignore me");
	eng->renderer()->set_driver(new r2::gl_render_driver(eng->renderer()));

	// Should work
	r2::scene* scene1 = scenes->create("scene1");
	assert(scene1 != nullptr);

	// Can't create two scenes with the same name
    r2::scene* scene2 = scenes->create("scene1");
	assert(scene2 == nullptr);

	// Also should work
    r2::scene* scene3 = scenes->create("scene2");
	assert(scene3 != nullptr);

	// Don't let the app run
	eng->window()->destroy();
    int r = eng->run();

	// Should work
    scenes->destroy(scene1);

	// Should error about null pointer passed
    scenes->destroy(scene2);

	// Should cause warning about undeleted scene (scene3 "scene2")
	eng->shutdown();
    return r;
}
