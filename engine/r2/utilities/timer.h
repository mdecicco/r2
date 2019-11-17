#pragma once
#include <r2/config.h>

namespace r2 {
	class timer {
		public:
			timer();
			~timer();

			void start();
			void pause();
			void reset();

			bool stopped() const { return m_stopped; }

			f32 elapsed() const;
			operator f32() const { return elapsed(); }
		protected:
			tmr::time_point m_startPoint;
			f32 m_pausedAt;
			bool m_stopped;
	};
};

