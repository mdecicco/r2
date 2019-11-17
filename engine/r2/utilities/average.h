#pragma once
#include <r2/utilities/timer.h>

namespace r2 {
	class average {
		public:
			average(u16 maxSamples);
			~average();

			u16 get_max_samples() const { return m_maxSamples; }
			u16 get_sample_count() const { return m_count; }

			void reset();

			f32 operator += (f32 sample);
			operator f32() const;

		protected:
			u16 m_maxSamples;
			f32* m_samples;
			f32 m_curAverage;
			f32 m_accumulated;
			u16 m_curSample;
			u16 m_count;
	};
};