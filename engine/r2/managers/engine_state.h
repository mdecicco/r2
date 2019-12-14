#pragma once
#include <r2/config.h>

#define TEMP_STATE_REF__ENGINE ((state*)0xC0CAC01A)

namespace r2 {
	class engine_state_data;
	class state;

	// Used by template functions
	void __set_temp_engine_state_ref(state* s);
	void __enable_state_mem();
	void __disable_state_mem();
	engine_state_data* __get_state_data(u16 factoryIdx);

	class engine_state_data {
		public:
			engine_state_data() : m_state(nullptr) { }
			virtual ~engine_state_data() { }

		protected:
			friend class state_man;
			state* m_state;
	};

	class engine_state_data_factory {
		public:
			engine_state_data_factory() { }
			~engine_state_data_factory() { }

			virtual engine_state_data* create() = 0;
	};

	template <typename T>
	class engine_state_data_ref {
		public:
			engine_state_data_ref()
				: m_factoryIdx(UINT16_MAX), m_enabled(0), m_valid(false) { }
			engine_state_data_ref(const engine_state_data_ref<T>& o)
				: m_factoryIdx(o.m_factoryIdx), m_enabled(o.m_enabled), m_valid(o.m_valid) { }

			~engine_state_data_ref() { }

			void enable() {
				assert(m_valid);
				if (m_enabled == 0) __enable_state_mem();
				m_enabled++;
			}

			void disable() {
				assert(m_valid);
				assert(m_enabled > 0);
				m_enabled--;
				if (m_enabled == 0) __disable_state_mem();
			}

			T* get() const {
				assert(m_valid);
				assert(m_enabled > 0);
				return (T*)__get_state_data(m_factoryIdx);
			}

			T* operator -> () const {
				assert(m_valid);
				assert(m_enabled > 0);
				return (T*)__get_state_data(m_factoryIdx);
			}

		protected:
			friend class state_man;
			engine_state_data_ref(u16 factoryIdx)
				: m_factoryIdx(factoryIdx), m_enabled(false), m_valid(true) { }

			u16 m_factoryIdx;
			u32 m_enabled;
			bool m_valid;
	};
};