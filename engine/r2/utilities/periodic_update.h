#pragma once
#include <r2/utilities/timer.h>
#include <r2/utilities/average.h>

namespace r2 {
	class periodic_update {
		public:
			periodic_update();
			~periodic_update();

			void initialize_periodic_update(u32 maxAverageDurSamples = 16);
			void destroy_periodic_update();
			void start_periodic_updates();
			void stop_periodic_updates();

			void setUpdateFrequency(f32 freq);
			f32 getAverageUpdateDuration() const;
			void update(f32 dt);

			virtual void doUpdate(f32 frameDelta, f32 updateDelta) = 0;
			virtual void belowFrequencyWarning(f32 percentLessThanDesired, f32 desiredFreq, f32 timeSpentLowerThanDesired) { }

		private:
			bool should_update();

			f32 m_updateFrequency;
			average* m_averageUpdateDuration;
			timer m_updateTmr;
			timer m_dt;
			timer m_timeSinceBelowFrequencyStarted;
			timer m_timeSinceBelowFrequencyLogged;
	};
};

