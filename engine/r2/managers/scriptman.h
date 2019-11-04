#pragma once
#include <r2/config.h>
#include <r2/managers/assetman.h>
#include <string>

#include <v8pp/context.hpp>
#include <v8pp/module.hpp>
#include <libplatform/libplatform.h>
#include <v8.h>

using namespace std;

namespace r2 {
	class r2engine;
	class script_man;
	class state;

	class script : public asset {
		public:
			script(state* parentState);
			~script();

			virtual bool deserialize(const unsigned char* data,size_t length);
			virtual bool serialize(unsigned char** data,size_t* length);

			bool valid() const { return m_is_valid; }

			void execute(bool logResult = false);

		protected:
			state* m_state;
			v8::Local<v8::Script> m_script;
			bool m_is_valid;
	};

	class script_env {
		public:
			script_env(r2engine* eng);
			~script_env();

			v8pp::context* context();
			v8::HandleScope* scope() { return &m_scope; }

		protected:
			r2engine* m_eng;
			v8::HandleScope m_scope;
	};

	class script_man {
		public:
			script_man(r2engine* eng);
			~script_man();

			void execute(const string& script);

			v8pp::context* context() { return &m_context; }

		protected:
			r2engine* m_engine;
			v8pp::context m_context;
			v8::HandleScope m_global_scope;
	};
};