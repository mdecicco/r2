#include <r2/utilities/timer.h>

namespace r2 {
	timer::timer() : m_stopped(true), m_pausedAt(0.0f) { }
	timer::~timer() { }

	void timer::start() {
		if (!m_stopped) return;
		m_startPoint = tmr::now();
		m_stopped = false;
	}
	void timer::pause() {
		if (m_stopped) return;
		m_pausedAt = elapsed();
		m_stopped = true;
	}
	void timer::reset() {
		m_stopped = true;
		m_pausedAt = 0.0f;
	}

	f32 timer::elapsed() const {
		if (m_stopped) return m_pausedAt;
		return m_pausedAt + dur(tmr::now() - m_startPoint).count();
	}
};