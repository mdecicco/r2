#pragma once
#include <r2/config.h>
#include <functional>

namespace r2 {
    typedef f32 (*InterpolationFactorCallback)(f32);
    
    namespace interpolate {
        // no easing, no acceleration
        f32 linear (f32 normalizedInput);
        // accelerating from zero velocity
        f32 easeInQuad (f32 normalizedInput);
        // decelerating to zero velocity
        f32 easeOutQuad (f32 normalizedInput);
        // acceleration until halfway, then deceleration
        f32 easeInOutQuad (f32 normalizedInput);
        // accelerating from zero velocity 
        f32 easeInCubic (f32 normalizedInput);
        // decelerating to zero velocity 
        f32 easeOutCubic (f32 normalizedInput);
        // acceleration until halfway, then deceleration 
        f32 easeInOutCubic (f32 normalizedInput);
        // accelerating from zero velocity 
        f32 easeInQuart (f32 normalizedInput);
        // decelerating to zero velocity 
        f32 easeOutQuart (f32 normalizedInput);
        // acceleration until halfway, then deceleration
        f32 easeInOutQuart (f32 normalizedInput);
        // accelerating from zero velocity
        f32 easeInQuint (f32 normalizedInput);
        // decelerating to zero velocity
        f32 easeOutQuint (f32 normalizedInput);
        // acceleration until halfway, then deceleration 
        f32 easeInOutQuint (f32 normalizedInput);
    };
    
    template <typename t>
    class Interpolator {
        public:
            Interpolator (const t& initial, f32 duration, InterpolationFactorCallback method)
			: m_initial(initial), m_final(initial), m_duration(duration), m_useFinishedCb(false), m_transitionMethod(method), m_isStopped(true) {
            }

            ~Interpolator () { }
            
            f32 duration () const { return m_duration; }

            void duration (f32 duration) { m_duration = duration; }

            void set_immediate (const t& value) {
                m_initial = value;
                m_final = value;
                m_useFinishedCb = false;
                m_isStopped = true;
            }

            void then (std::function<void()> cb) {
                m_useFinishedCb = true;
                m_finished = cb;
            }
            
            Interpolator& operator = (const t& value) {
                if(!m_isStopped) m_initial = *this;
                else m_useFinishedCb = false;
				m_startTick = tmr::now();
				m_isStopped = false;
                m_final = value;
                return *this;
            }

            operator t() {
                if(m_isStopped) return m_final;
				f32 elapsed = dur(tmr::now() - m_startTick).count();
                if(elapsed >= m_duration) {
					m_isStopped = true;
                    m_initial = m_final;
                    if(m_useFinishedCb) {
                        m_useFinishedCb = false;
                        m_finished();
                    }
                    return m_initial;
                }
                f32 factor = m_transitionMethod(elapsed / m_duration);
                return m_initial + ((m_final - m_initial) * factor);
            }
        
        protected:
            InterpolationFactorCallback m_transitionMethod;
            bool m_useFinishedCb;
            std::function<void()> m_finished;
            t m_final;
            t m_initial;
            tmr::time_point m_startTick;
			bool m_isStopped;
            f32 m_duration;
    };
};
