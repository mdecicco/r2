#include <r2/engine.h>
using namespace r2;

class test_receiver : public event_receiver {
    public:
        test_receiver() {}
        virtual ~test_receiver() {}

        virtual void handle(event* e) {
            i32 i = 0;
            printf("Test\n");
            if(!e->data()->read_int32(i)) {
                printf("Failed to read int from event\n");
                return;
            }

			assert(i == 4321);
            printf("event received: %s: %d\n", e->data()->name().c_str(), i);
        }
};

int main(int argc, char** argv) {
	r2::r2engine::create(argc, argv);
	r2::r2engine* eng = r2::r2engine::get();

    test_receiver* rec = new test_receiver();
    eng->add_child(rec);

    event* e = new evt("test_event", true);
    e->data()->write_int32(4321);
    eng->dispatch(e);

    int r = eng->run();

	delete e;
    delete rec;
	eng->shutdown();

    return r;
}
