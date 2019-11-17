#include <r2/managers/scriptman.h>
#include <r2/engine.h>
#include <r2/bindings/bindings.h>
#include <iostream>

namespace r2 {
	script_man::script_man(r2engine* eng) : m_engine(eng), m_global_scope(m_context.isolate()) {
		m_context.set_lib_path("./resources");
		bind_engine(&m_context);
	}

	script_man::~script_man() {
	}

	void script_man::execute(const string& source) {
		v8pp::context context;
		auto isolate = context.isolate();
		v8::HandleScope handleScope(isolate);

		try {
			v8::TryCatch exc(isolate);
			auto result = context.run_script(source);
			if (exc.HasCaught()) {
				v8::Local<v8::Message> message = exc.Message();
				v8::Local<v8::Context> v8context = isolate->GetCurrentContext();
				v8::String::Utf8Value sourceline(isolate, message->GetSourceLine(v8context).ToLocalChecked());
				v8::String::Utf8Value str(isolate, exc.Exception());
				r2Error("Exception (line %d):\n%s\n%s\n", message->GetLineNumber(v8context).FromJust(), *sourceline, *str);
			} else {
				v8::String::Utf8Value resultString(isolate, result);
				std::cout << "The result of the script was: " << *resultString << "\n";
			}
		} catch (std::exception& e) {
			std::cerr << e.what() << std::endl;
		}
	}

	void script_man::executeFile(const string& file) {
		auto f = r2engine::get()->files()->open(file, DM_TEXT, file);
		if (!f) {
			r2Error("Failed to open script \"%s\"\n", file.c_str());
			return;
		}
		string source;
		source.resize(f->size());
		f->read_data(&source[0], f->size());
		r2engine::get()->files()->destroy(f);

		auto isolate = m_context.isolate();
		v8::Local<v8::Context> context = isolate->GetCurrentContext();

		v8::ScriptOrigin origin(v8pp::convert<string>::to_v8(isolate, file));
		v8::Local<v8::Script> script;
		bool is_valid = v8::Script::Compile(context, v8pp::convert<string>::to_v8(isolate, source), &origin).ToLocal(&script);

		if (!is_valid) return;

		if (!script.IsEmpty()) script->Run(context);
		else r2Error("Script \"%s\" is empty", file.c_str());
	}

	script_env::script_env() : m_scope(context()->isolate()) {
	}
	script_env::~script_env() {
	}
	v8pp::context* script_env::context() {
		return r2engine::get()->scripts()->context();
	}

	script::script(state* parentState) : m_state(parentState), m_is_valid(false) {
	}

	script::~script() {
	}

	bool script::deserialize(const unsigned char* data, size_t length) {
		string source = string((const char*)data, length);

		auto ctx = r2engine::get()->scripts()->context();
		
		v8::ScriptOrigin origin(v8pp::to_v8(ctx->isolate(), m_filename));
		v8::Local<v8::Context> context = ctx->isolate()->GetCurrentContext();
		m_is_valid = v8::Script::Compile(context, v8pp::to_v8(ctx->isolate(), source), &origin).ToLocal(&m_script);
		
		return true;
	}

	bool script::serialize(unsigned char** data,size_t* length) {
		return false;
	}

	void script::execute(bool logResult) {
		try {
			if (!m_script.IsEmpty()) {
				auto ctx = r2engine::get()->scripts()->context();
				auto isolate = ctx->isolate();
				v8::Local<v8::Context> v8context = isolate->GetCurrentContext();
				v8::TryCatch exc(isolate);
				
				v8::Local<v8::Value> result;
				bool success = m_script->Run(v8context).ToLocal(&result);

				if (exc.HasCaught()) {
					v8::Local<v8::Message> message = exc.Message();
					v8::String::Utf8Value sourceline(isolate, message->GetSourceLine(v8context).ToLocalChecked());
					v8::String::Utf8Value str(isolate, exc.Exception());
					r2Error("Exception (line %d):\n%s\n%s", message->GetLineNumber(v8context).FromJust(), *sourceline, *str);
				} else if (logResult) {
					v8::String::Utf8Value resultString(isolate, result);
					r2Log("Script output: %s", *resultString);
				}
			}
		} catch (std::exception& e) {
			std::cerr << e.what() << std::endl;
		}
	}
};
