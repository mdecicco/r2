#include <r2/engine.h>

namespace r2 {
	state* temp_state_ref = nullptr;
	void __set_temp_engine_state_ref(state* s) {
		temp_state_ref = s;
	}
	void __enable_state_mem() {
		state* current = temp_state_ref ? temp_state_ref : r2engine::get()->states()->current();
		if (!current || current == TEMP_STATE_REF__ENGINE) {
			memory_man::push_current(memory_man::global());
			return;
		}

		current->activate_allocator();
	}

	void __disable_state_mem() {
		state* current = temp_state_ref ? temp_state_ref : r2engine::get()->states()->current();
		if (!current || current == TEMP_STATE_REF__ENGINE) {
			memory_man::pop_current();
			return;
		}

		current->deactivate_allocator();
	}

	engine_state_data* __get_state_data(u16 factoryIdx) {
		state* current = temp_state_ref ? temp_state_ref : r2engine::get()->states()->current();
		if (!current || current == TEMP_STATE_REF__ENGINE) return r2engine::get()->get_engine_state_data(factoryIdx);
		return current->get_engine_state_data(factoryIdx);
	}
	engine_state_data_factory* __get_state_factory(u16 factoryIdx) {
		return r2engine::states()->factory(factoryIdx);
	}
};