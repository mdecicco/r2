#include <r2/managers/interpolationman.h>

namespace r2 {
	interpolation_man::interpolation_man() {
		m_last_time = 0.0f;

		m_interpolators.next = nullptr;
		m_interpolators.interp = nullptr;

		size_t max_count = 1024;
		m_list_mem_base = new ilist[max_count];
		ilist* cur = &m_free_list;
		for (size_t i = 0;i < max_count;i++) {
			cur->interp = nullptr;
			cur->next = &m_list_mem_base[i];
			cur = cur->next;
		}

		cur->interp = nullptr;
		cur->next = nullptr;
	}

	interpolation_man::~interpolation_man() {
		clear();
		delete [] m_list_mem_base;
		m_list_mem_base = nullptr;
	}


	void interpolation_man::cancel(remote_interpolator_base* interpolation, bool setToEndValue) {
		ilist* last = &m_interpolators;
		ilist* cur = m_interpolators.next;
		while (cur) {
			if (cur->interp == interpolation) {
				m_interpolatorMap.erase(interpolation->remote_value_ptr());
				interpolation->cancel(setToEndValue);

				last->next = cur->next;
				delete cur->interp;
				cur->interp = nullptr;
				cur->next = m_free_list.next;
				m_free_list.next = cur;
				return;
			}
			last = cur;
			cur = cur->next;
		}
	}

	void interpolation_man::clear() {
		while (m_interpolators.next) {
			delete m_interpolators.next->interp;
			m_interpolators.next->interp = nullptr;
			
			ilist* next = m_interpolators.next->next;
			m_interpolators.next->next = m_free_list.next;
			m_free_list.next = m_interpolators.next;
			m_interpolators.next = next;
		}
	}

	void interpolation_man::update() {
		if (m_timer.stopped()) {
			m_timer.start();
			return;
		}
		f32 ct = m_timer;
		f32 dt = ct - m_last_time;
		m_last_time = ct;

		ilist* last = &m_interpolators;
		ilist* cur = m_interpolators.next;
		while (cur) {
			if (cur->interp->update(dt)) {
				last->next = cur->next;
				delete cur->interp;
				cur->interp = nullptr;
				cur->next = m_free_list.next;
				m_free_list.next = cur;
				cur = last->next;
			} else {
				last = cur;
				cur = cur->next;
			}
		}
	}
};
