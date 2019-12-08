#include <r2/bindings/bindings.h>
#include <r2/engine.h>
#include <v8pp/class.hpp>

using namespace std;
using namespace v8;
using namespace v8pp;

namespace r2 {
	void bind_event(context* ctx) {
		auto isolate = ctx->isolate();
		auto context = isolate->GetCurrentContext();
		auto global = context->Global();

		class_<event, v8pp::raw_ptr_traits> s(isolate);
		register_class_state(s);
		s.ctor<v8Args>();
		s.auto_wrap_objects(true);
		s.set("name", property(&event::name));
		s.set("stop_propagation", &event::stop_propagating);
		s.set("data", property(&event::get_json, &event::set_json));
		ctx->set("Event", s);
	}

	void release_event_objects() {
		Isolate* i = r2engine::get()->scripts()->context()->isolate();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<event>()).remove_objects();
	}

	void reset_event_object_storage() {
		Isolate* i = r2engine::get()->scripts()->context()->isolate();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<event>()).reset_objects_map();
	}
};