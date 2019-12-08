#include <r2/engine.h>
using namespace r2;
class test_component : public scene_entity_component {
	public:
		test_component(i32 _x, i32 _y, i32 _z) : x(_x), y(_y), z(_z) { }
		~test_component() { }

		i32 x;
		i32 y;
		i32 z;
};

class test_system : public entity_system {
	public:
		test_system() { }
		~test_system() { }

		virtual const size_t component_size() const { return sizeof(test_component); }
		virtual const size_t max_component_count() const { return 3; }

		virtual void initialize_entity(scene_entity* entity) {
			entity->bind("test_callback");

			LocalValueHandle args[2] = {
				v8::Number::New(r2engine::isolate(), 12.34),
				v8::String::NewFromUtf8(r2engine::isolate(), "testy", v8::NewStringType::kNormal).ToLocalChecked()
			};

			entity->bind(this, "add_test_component", [](entity_system* system, scene_entity* entity, v8Args args) {
				system->addComponentTo(entity);
			});

			entity->call("test_callback", 2, args);
		}
		virtual void deinitialize_entity(scene_entity* entity) {
			entity->unbind("add_test_component");
		}

		virtual scene_entity_component* create_component(entityId id) {
			state().enable();
			auto out = state()->create<test_component>(id, 420, 69, 12);
			state().disable();
			return out;
		}

		virtual void bind(scene_entity_component* component, scene_entity* entity) {
			using c = test_component;
			entity->bind(component, "x", &c::x);
			entity->bind(component, "y", &c::y);
			entity->bind(component, "z", &c::z);
			entity->bind(this, "test_component_id", [](entity_system* system, scene_entity* entity, v8Args args) {
				system->state().enable();
				auto component = system->state()->entity(entity->id());
				args.GetReturnValue().Set(v8pp::convert<componentId>::to_v8(args.GetIsolate(), component->id()));
				system->state().disable();
			});
			entity->bind(this, "remove_test_component", [](entity_system* system, scene_entity* entity, v8Args args) {
				system->removeComponentFrom(entity);
			});
		}
		virtual void unbind(scene_entity* entity) {
			entity->unbind("remove_test_component");
			entity->unbind("x");
			entity->unbind("y");
			entity->unbind("z");
		}

		virtual void initialize() {
		}

		virtual void tick(f32 dt) {
			state().enable();
			state()->for_each<test_component>([](test_component* component) {
				component->x++;
				component->y--;
				component->z = component->x - component->y;
			});
			state().disable();
		}

		virtual void handle(event* evt) {
		}
};

int main(int argc, char** argv) {
	r2engine::register_system(new test_system());
	r2engine::create(argc, argv);
	r2engine::get()->scripts()->executeFile("./resource/entity_test/entity_test.js");

	int ret = r2engine::get()->run();

	r2engine::shutdown();
	return 0;
}
