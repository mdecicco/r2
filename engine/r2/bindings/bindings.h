#pragma once
namespace v8pp {
	class context;
};

namespace r2 {
	void bind_engine(v8pp::context* ctx);
	void release_state_objects();
	void reset_state_object_storage();

	void bind_event(v8pp::context* ctx);
	void release_event_objects();
	void reset_event_object_storage();

	void bind_math(v8pp::context* ctx);
	void release_math_objects();
	void reset_math_object_storage();

	void bind_imgui(v8pp::context* ctx);
	void release_imgui_objects();
	void reset_imgui_object_storage();

	void bind_graphics(v8pp::context* ctx);
	void release_graphics_objects();
	void reset_graphics_object_storage();

	void bind_io(v8pp::context* ctx);
	void release_io_objects();
	void reset_io_object_storage();
};