#include <r2/utilities/periodic_update.h>

namespace r2 {
	periodic_update::periodic_update() : m_averageUpdateDuration(nullptr), m_updateFrequency(0.0f) { }

	periodic_update::~periodic_update() {
		if (m_averageUpdateDuration) delete m_averageUpdateDuration;
	}

	void periodic_update::initialize_periodic_update(u32 maxAverageDurSamples) {
		destroy_periodic_update();
		m_averageUpdateDuration = new average(maxAverageDurSamples);
	}

	void periodic_update::destroy_periodic_update() {
		if (m_averageUpdateDuration) delete m_averageUpdateDuration;
		m_averageUpdateDuration = nullptr;
	}

	void periodic_update::start_periodic_updates() {
		m_updateTmr.reset();
		m_updateTmr.start();
		m_dt.reset();
		m_dt.start();
	}

	void periodic_update::stop_periodic_updates() {
		m_updateTmr.reset();
		m_dt.reset();
	}

	void periodic_update::setUpdateFrequency(f32 freq) {
		m_updateFrequency = freq;
	}

	f32 periodic_update::getAverageUpdateDuration() const {
		return *m_averageUpdateDuration;
	}

	void periodic_update::update(f32 dt) {
		if(!should_update()) return;
		f32 update_dt = m_dt;
		m_dt.reset();
		m_dt.start();
		doUpdate(dt, update_dt);
		if (m_averageUpdateDuration) {
			// doUpdate could have resulted in a call to destroy_periodic_update
			(*m_averageUpdateDuration) += (f32)m_dt;
		}
	}

	bool periodic_update::should_update() {
		if (m_updateFrequency <= 0.0f) return true;
		f32 timeSinceUpdate = m_updateTmr;
		f32 waitDuration = (1.0f / m_updateFrequency);
		static f32 warnDiffFrac = 0.1f;
		static f32 warnLogInterval = 30.0f;
		static f32 warnAfterBelowForInterval = 5.0f;
		if (timeSinceUpdate >= waitDuration) {
			f32 delta = timeSinceUpdate - waitDuration;
			if (delta > waitDuration * warnDiffFrac) {
				if (m_timeSinceBelowFrequencyStarted.stopped()) {
					m_timeSinceBelowFrequencyStarted.start();
				}
				bool warnCondition0 = m_timeSinceBelowFrequencyStarted.elapsed() > warnAfterBelowForInterval;
				bool warnCondition1 = m_timeSinceBelowFrequencyLogged.elapsed() > warnLogInterval || m_timeSinceBelowFrequencyLogged.stopped();
				if (warnCondition0 && warnCondition1) {
					belowFrequencyWarning(warnDiffFrac * 100.0f, m_updateFrequency, warnAfterBelowForInterval);
					
					if (m_timeSinceBelowFrequencyLogged.stopped()) m_timeSinceBelowFrequencyLogged.start();
					else {
						m_timeSinceBelowFrequencyLogged.reset();
						m_timeSinceBelowFrequencyLogged.start();
					}
				}
			} else if (!m_timeSinceBelowFrequencyStarted.stopped()) {
				m_timeSinceBelowFrequencyStarted.reset();
			}

			m_updateTmr.reset();
			m_updateTmr.start();
			return true;
		}

		return false;
	}
};
