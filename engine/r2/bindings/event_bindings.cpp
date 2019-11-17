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

		class_<event, raw_ptr_traits> s(isolate);
		s.ctor<v8Args>();
		s.set("name", property(&event::name));
		s.set("stop_propagation", &event::stop_propagating);
		s.set("data", property(&event::get_script_data, &event::set_script_data));
		ctx->set("Event", s);
	}
};