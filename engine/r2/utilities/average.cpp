#include <r2/utilities/average.h>

namespace r2 {
	average::average(u16 maxSamples) : m_maxSamples(maxSamples), m_curSample(0), m_count(0), m_samples(0), m_curAverage(0), m_accumulated(0) {
		m_samples = new f32[m_maxSamples];
		memset(m_samples, 0, m_maxSamples * sizeof(f32));
	}
	average::~average() {
		if (m_samples) delete [] m_samples;
	}

	void average::reset() {
		memset(m_samples, 0, m_maxSamples * sizeof(f32));
		m_accumulated = 0;
		m_curAverage = 0;
		m_curSample = 0;
		m_count = 0;
	}

	f32 average::operator += (f32 sample) {
		m_accumulated += sample;
		if (m_count < m_maxSamples) m_count++;
		else {
			m_accumulated -= m_samples[m_curSample];
			m_samples[m_curSample++] = sample;
			if (m_curSample == m_maxSamples) m_curSample = 0;
		}
		f32 accum = m_samples[0];
		for(u16 i = 1;i < m_count;i++) accum += m_samples[i];
		return m_curAverage = accum / f32(m_count);

		// pretty sure this works, just want to be sure for now
		//return m_curAverage = m_accumulated / f32(m_count);
	}

	average::operator r2::f32() const {
		return m_curAverage;
	}
};