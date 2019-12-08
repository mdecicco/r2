#include <r2/engine.h>

#include <r2/bindings/math_defs.h>
#include <r2/utilities/average.h>

#include <v8pp/context.hpp>
#include <v8pp/module.hpp>
#include <v8pp/class.hpp>

#include <r2/bindings/math_converters.h>

#include <functional>

using namespace v8;
using namespace v8pp;
using namespace std;

namespace r2 {
	void register_structs(context* ctx) {
		auto isolate = ctx->isolate();
		auto global = isolate->GetCurrentContext()->Global();

		//average
		{
			class_<average, v8pp::raw_ptr_traits> s(global->GetIsolate());
			register_class_state(s);
			s.ctor<u16>();
			s.set("sample", &average::operator+=);
			s.set("reset", &average::reset);
			s.set("max_samples", v8pp::property(&average::get_max_samples));
			s.set("sample_count", v8pp::property(&average::get_sample_count));
			s.set("average", v8pp::property(&average::operator f32));
			ctx->set("RollingAverage", s);
		}
	}

	void bind_math(context* ctx) {
		auto isolate = ctx->isolate();
		auto global = isolate->GetCurrentContext()->Global();
		register_structs(ctx);
	}

	void release_math_objects() {
		Isolate* i = r2engine::get()->scripts()->context()->isolate();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<average>()).remove_objects();
	}

	void reset_math_object_storage() {
		Isolate* i = r2engine::get()->scripts()->context()->isolate();
		detail::classes::find<raw_ptr_traits>(i, detail::type_id<average>()).reset_objects_map();
	}
};