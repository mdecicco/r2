#include <r2/managers/scriptman.h>
#include <r2/engine.h>
#include <r2/bindings/bindings.h>

#include <algorithm>

#include <v8.h>
#include <r2/bindings/v8helpers.h>
using namespace v8;
using namespace v8pp;

namespace r2 {
	class ArrayBufferAllocator : public v8::ArrayBuffer::Allocator {
		public:
			virtual void* Allocate(size_t length) {
				void* data = AllocateUninitialized(length);
				return data == NULL ? data : memset(data, 0, length);
			}
			virtual void* AllocateUninitialized(size_t length) { return r2alloc(length); }
			virtual void Free(void* data, size_t) { r2free(data); }
	};

	mstring get_contents(const mstring& file) {
		mstring contents;
		FILE* fp = fopen(file.c_str(), "rb");
		if (!fp) {
			r2Error("File %s not found.", file.c_str());
		} else {
			fseek(fp, 0, SEEK_END);
			size_t fsz = ftell(fp);
			fseek(fp, 0, SEEK_SET);
			contents.resize(fsz);

			if (fsz > 0) {
				if (fread(&contents[0], fsz, 1, fp) != 1) {
					r2Error("Failed to read %s", file.c_str());
					contents.resize(0);
				}
			} else {
				r2Error("%s is empty", file.c_str());
				contents.resize(0);
			}
			fclose(fp);
		}
		return contents;
	}

	static inline void ltrim(mstring &s) {
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
			return !std::isspace(ch);
		}));
	}

	// trim from end (in place)
	static inline void rtrim(mstring &s) {
		s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
			return !std::isspace(ch);
		}).base(), s.end());
	}

	// trim from both ends (in place)
	static inline void trim(mstring &s) {
		ltrim(s);
		rtrim(s);
	}

	void script_exception(Isolate* isolate, Handle<Value> er, Handle<Message> message) {
		HandleScope scope(isolate);

		mstring trace;
		if (er->IsUndefined() || er->IsNull()) trace = "(no trace available)";
		else {
			Local<Object> obj = er->ToObject(isolate);
			if (obj->IsUndefined()) trace = "(no trace available)";
			else {
				Local<Value> tvalue = obj->Get(String::NewFromUtf8(isolate, "stack"));
				if (tvalue.IsEmpty() || tvalue->IsUndefined()) trace = "(no trace available)";
				else trace = *String::Utf8Value(isolate, tvalue);
			}
		}

		if (!message.IsEmpty()) {
			mstring line_str = *String::Utf8Value(isolate, message->GetSourceLine(isolate->GetCurrentContext()).ToLocalChecked());
			const char* file = *String::Utf8Value(isolate, message->GetScriptResourceName());
			i32 line = 0;
			i32 ch = message->GetEndColumn();
			message->GetLineNumber(isolate->GetCurrentContext()).To(&line);
			for(u16 i = 0;i < line_str.length();i++) {
				if (isspace(line_str[i])) ch--;
				else break;
			}
			trim(line_str);

			mstring err = line_str + '\n';
			for(u16 i = 0;i < ch - 1;i++) err += ' ';
			err += "^\n";
			err += trace;
			r2Error(err.c_str());
		} else {
			r2Error("Uh oh: %s", trace.c_str());
		}
	}
	void script_exception(Isolate* isolate, TryCatch& tc) {
		HandleScope scope(isolate);

		Handle<Value> er = tc.Exception();
		Handle<Message> message = tc.Message();

		mstring trace;
		if (er->IsUndefined() || er->IsNull()) trace = "(no trace available)";
		else {
			Local<Object> obj = er->ToObject(isolate);
			if (obj->IsUndefined()) trace = "(no trace available)";
			else {
				Local<Value> tvalue = obj->Get(String::NewFromUtf8(isolate, "stack"));
				if (tvalue.IsEmpty() || tvalue->IsUndefined()) trace = "(no trace available)";
				else trace = *String::Utf8Value(isolate, tvalue);
			}
		}

		if (!message.IsEmpty()) {
			mstring line_str = *String::Utf8Value(isolate, message->GetSourceLine(isolate->GetCurrentContext()).ToLocalChecked());
			const char* file = *String::Utf8Value(isolate, message->GetScriptResourceName());
			i32 line = 0;
			i32 ch = message->GetEndColumn();
			message->GetLineNumber(isolate->GetCurrentContext()).To(&line);
			for(u16 i = 0;i < line_str.length();i++) {
				if (isspace(line_str[i])) ch--;
				else break;
			}
			trim(line_str);

			mstring err = line_str + '\n';
			for(u16 i = 0;i < ch - 1;i++) err += ' ';
			err += "^\n";
			err += trace;
			r2Error(err.c_str());
		} else {
			r2Error("Uh oh: %s", trace.c_str());
		}
	}

	bool check_script_exception(Isolate* isolate, const TryCatch& tc) {
		if (tc.HasCaught()) {
			script_exception(isolate, tc.Exception(), tc.Message());
			return false;
		}
		return true;
	}

	void js_require(v8Args args) {
		Isolate* i = args.GetIsolate();
		if (args.Length() != 1 || !args[0]->IsString()) {
			r2Error("Invalid argument(s?) provided to require.");
			return;
		}

		mstring file = convert<mstring>::from_v8(i, args[0]);

		auto bs = file.find_first_of('\\');
		while(bs != mstring::npos) {
			file.replace(bs, bs, "/");
			bs = file.find_first_of('\\');
		}

		trace t(i);

		size_t len = file.length();
		mstring end = file.substr(len - 3);
		std::transform(end.begin(), end.end(), end.begin(), ::tolower);

		if (end != ".js") file += ".js";

		mstring current_dir = t.file.substr(0, t.file.find_last_of('/') + 1);
		mstring require_path = current_dir + file;
		mstring source = get_contents(require_path);
		if (source.length() == 0) return;

		if (source.find("//r2-do-not-wrap") == string::npos) {
			auto exportIdx = source.rfind("export");
			if (exportIdx == mstring::npos) {
				r2Log("Module \"%s\" does not export anything", require_path.c_str());
			} else {
				mstring s = source.substr(exportIdx);
				source.replace(exportIdx, 6, "return");
				source.insert(0, "(function() {");
				source += "})();";
			}
		}

		Local<Context>& context = i->GetCurrentContext();

		EscapableHandleScope scope(i);
		TryCatch tc(i);
		ScriptOrigin origin(convert<mstring>::to_v8(i, require_path));
		Local<Script> script;
		bool is_valid = Script::Compile(context, convert<mstring>::to_v8(i, source), &origin).ToLocal(&script);

		if (tc.HasCaught()) script_exception(i, tc.Exception(), tc.Message());
		if (!is_valid) { return; }

		if (!script.IsEmpty()) {
			Local<Value> localResult;

			MaybeLocal<Value> result = script->Run(context);
			if (tc.HasCaught()) {
				script_exception(i, tc.Exception(), tc.Message());
				args.GetReturnValue().Set(scope.Escape(localResult));
				return;
			} else if(!result.ToLocal(&localResult)) r2Error("Failed to get exports from module \"%s\"", require_path.c_str());

			args.GetReturnValue().Set(scope.Escape(localResult));
		} else r2Error("Module \"%s\" is empty", require_path.c_str());
	}

	script_man::script_man() : m_context(new v8pp::context(nullptr, new ArrayBufferAllocator(), false)), m_global_scope(m_context->isolate()) {
	}

	script_man::~script_man() {
		// purposely not deleting m_context.
		// v8pp::context will attempt to deallocate objects that are managed by the engine
		// The memory allocated by the context and the memory allocated for the context itself
		// will be deallocated when memory_man::instance is deallocated
	}

	void script_man::initialize() {
		bind_engine(m_context);
		m_context->set("require", wrap_function(m_context->isolate(), "require", &js_require));
	}

	bool script_man::execute(const mstring& source, v8::Local<v8::Value>* result) {
		if (source.length() == 0) return false;

		auto isolate = m_context->isolate();
		Local<Context> context = isolate->GetCurrentContext();

		TryCatch try_catch(isolate);
		try_catch.SetVerbose(false);
		ScriptOrigin origin(convert<mstring>::to_v8(isolate, "command"), Local<Integer>::New(isolate, Integer::New(isolate, 0)));
		Local<Script> script;
		bool is_valid = Script::Compile(context, convert<mstring>::to_v8(isolate, source), &origin).ToLocal(&script);

		if (try_catch.HasCaught()) {
			script_exception(isolate, try_catch.Exception(), try_catch.Message());
			return false;
		}

		if (!is_valid) return false;

		if (!script.IsEmpty()) {
			if (result) {
				EscapableHandleScope scope(isolate);
				MaybeLocal<Value> ret = script->Run(context);
				scope.EscapeMaybe(ret).ToLocal(result);
			} else script->Run(context);

			if (try_catch.HasCaught()) {
				script_exception(isolate, try_catch.Exception(), try_catch.Message());
				return false;
			}
		} else {
			r2Error("Command is empty");
			return false;
		}

		return true;
	}

	bool script_man::executeFile(const mstring& file, v8::Local<v8::Value>* result) {
		mstring source = get_contents(file);
		if (source.length() == 0) return false;

		auto isolate = m_context->isolate();
		Local<Context> context = isolate->GetCurrentContext();

		TryCatch try_catch(isolate);
		try_catch.SetVerbose(true);
		ScriptOrigin origin(convert<mstring>::to_v8(isolate, file), Local<Integer>::New(isolate, Integer::New(isolate, 0)));
		Local<Script> script;
		bool is_valid = Script::Compile(context, convert<mstring>::to_v8(isolate, source), &origin).ToLocal(&script);

		if (try_catch.HasCaught()) {
			script_exception(isolate, try_catch);
			return false;
		}

		if (!is_valid) return false;

		if (!script.IsEmpty()) {
			if (result) {
				EscapableHandleScope scope(isolate);
				MaybeLocal<Value> ret = script->Run(context);
				scope.EscapeMaybe(ret).ToLocal(result);
			} else script->Run(context);

			if (try_catch.HasCaught()) {
				script_exception(isolate, try_catch);
				return false;
			}
		} else {
			r2Error("Script \"%s\" is empty", file.c_str());
			return false;
		}

		return true;
	}



	script::script(state* parentState) : m_state(parentState), m_is_valid(false) {
		// todo: make m_script persistent
	}

	script::~script() {
		// todo: free persistent m_script
	}

	bool script::deserialize(const unsigned char* data, size_t length) {
		mstring source = mstring((const char*)data, length);

		auto ctx = r2engine::get()->scripts()->context();

		TryCatch try_catch(ctx->isolate());
		ScriptOrigin origin(to_v8(ctx->isolate(), m_filename));
		Local<Context> context = ctx->isolate()->GetCurrentContext();
		m_is_valid = Script::Compile(context, to_v8(ctx->isolate(), source), &origin).ToLocal(&m_script);
		if (try_catch.HasCaught()) script_exception(ctx->isolate(), try_catch.Exception(), try_catch.Message());
		
		return true;
	}

	bool script::serialize(unsigned char** data, size_t* length) {
		return false;
	}

	void script::execute(bool logResult) {
		if (!m_script.IsEmpty()) {
			auto ctx = r2engine::get()->scripts()->context();
			auto isolate = ctx->isolate();
			Local<Context> v8context = isolate->GetCurrentContext();
			TryCatch try_catch(isolate);
				
			var result;
			result.isolate = isolate;
			bool success = m_script->Run(v8context).ToLocal(&result.value);

			if (try_catch.HasCaught()) script_exception(ctx->isolate(), try_catch.Exception(), try_catch.Message());
			else if (logResult) r2Log("Script output: %s", string(result).c_str());
		}
	}
};
