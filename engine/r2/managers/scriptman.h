#pragma once
#include <r2/config.h>
#include <r2/managers/assetman.h>
#include <r2/managers/memman.h>

#include <v8pp/context.hpp>
#include <v8pp/module.hpp>
#include <libplatform/libplatform.h>
#include <v8.h>

using namespace std;

namespace r2 {
	class r2engine;
	class script_man;
	class state;

	bool check_script_exception(v8::Isolate* isolate, const v8::TryCatch& tc);

	class script : public asset {
		public:
			script(state* parentState);
			~script();

			virtual bool deserialize(const unsigned char* data, size_t length);
			virtual bool serialize(unsigned char** data, size_t* length);

			bool valid() const { return m_is_valid; }

			void execute(bool logResult = false);

		protected:
			state* m_state;
			v8::Local<v8::Script> m_script;
			bool m_is_valid;
	};

	class script_man {
		public:
			script_man();
			~script_man();

			void initialize();

			bool execute(const mstring& script, v8::Local<v8::Value>* result = nullptr);
			bool executeFile(const mstring& file, v8::Local<v8::Value>* result = nullptr);

			v8pp::context* context() { return m_context; }

		protected:
			v8pp::context* m_context;
			v8::HandleScope m_global_scope;
	};
};