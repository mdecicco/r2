#include <r2/bindings/bindings.h>
#include <r2/engine.h>

using namespace std;
using namespace v8;
using namespace v8pp;

#define v8str(str) v8::String::NewFromUtf8(isolate, str, v8::String::kNormalString, strlen(str))

namespace r2 {
	void bind_event_class(Local<Object> global) {
		auto isolate = global->GetIsolate();

		auto construct = [](v8Args args) {
			auto isolate = args.GetIsolate();
			if (!args.IsConstructCall()) {
				isolate->ThrowException(v8str("Cannot call constructor as function"));
				return;
			}

			HandleScope scope(isolate);
			trace t(isolate);
			
			// get arguments
			string name = var(isolate, args[0]);
			bool is_recursive = true;
			string data = "";
			if (args.Length() == 2) data = var(isolate, args[1]);
			if (args.Length() == 3) is_recursive = var(isolate, args[2]);

			// Read and pass constructor arguments
			auto obj = make_object<event>(args.This(), t.file, t.line, r2engine::get(), name, data.length() > 0, is_recursive);

			if (data.length() > 0) {
				auto wrap = Local<External>::Cast(obj.Get(isolate)->GetInternalField(0));
				event* e = static_cast<event*>(wrap->Value());
				e->data()->write_string(data);
			}

			args.GetReturnValue().Set(args.This());
		};

		auto tpl = FunctionTemplate::New(isolate, construct);

		tpl->SetClassName(v8str("Event"));
		tpl->InstanceTemplate()->SetInternalFieldCount(1);

		// Add object properties to the prototype
		// Methods, Properties, etc.
		auto name_getter = [](v8Args args) {
			auto isolate = args.GetIsolate();
			auto s = unwrap<event>(args);
			args.GetReturnValue().Set(v8str(s->name().c_str()));
		};
		auto stop_propagation = [](v8Args args) {
			auto isolate = args.GetIsolate();
			auto s = unwrap<event>(args);
			s->stop_propagating();
		};
		auto data_getter = [](v8Args args) {
			auto isolate = args.GetIsolate();
			auto s = unwrap<event>(args);
			auto data = s->data();

			if (args.Length() == 1) {
				if (args[0]->IsObject()) {
					if (data) data->clear();
					else data = r2engine::get()->files()->create(DM_TEXT);
					data->write_string(var(isolate, args[0]));
					data->set_position(0);
				} else {
					isolate->ThrowException(v8str("Event::data() was passed something that wasn't an object."));
				}
			} else if (args.Length() > 1) {
				isolate->ThrowException(v8str("Event::data() accepts only one argument or zero arguments."));
			}

			if (data) {
				data->set_position(0);
				string s;
				s.resize(data->size());
				data->read_data(&s[0], data->size());
				args.GetReturnValue().Set(var(isolate, s).value);
			}
		};

		auto prototype = tpl->PrototypeTemplate();
		prototype->Set(v8str("name"), FunctionTemplate::New(isolate, name_getter));
		prototype->Set(v8str("stop_propagation"), FunctionTemplate::New(isolate, stop_propagation));
		prototype->Set(v8str("data"), FunctionTemplate::New(isolate, data_getter));
		

		auto cfn = tpl->GetFunction(isolate->GetCurrentContext());
		global->Set(v8str("Event"), cfn.ToLocalChecked());
	}

	void new_event(const FunctionCallbackInfo<Value>& args) {
		auto isolate = args.GetIsolate();
		auto context = isolate->GetCurrentContext();
		auto global = context->Global();
		trace t(isolate);
		string name = var(isolate, args[0]);
		bool is_recursive = true;
		string data = "";
		if (args.Length() == 2) data = var(isolate, args[1]);
		if (args.Length() == 3) is_recursive = var(isolate, args[2]);
		event* e = new event(t.file, t.line, r2engine::get(), name, data.length() > 0, is_recursive);

		if (data.length() > 0) {
			e->data()->write_string(data);
			e->data()->set_position(0);
		}
	}

	void bind_event(r2engine* eng, context* ctx) {
		auto isolate = ctx->isolate();
		auto context = isolate->GetCurrentContext();
		auto global = context->Global();

		bind_event_class(global);
	}
};