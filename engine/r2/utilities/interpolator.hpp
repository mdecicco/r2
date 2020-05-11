#pragma once
#include <r2/config.h>
#include <functional>

namespace r2 {
    typedef f32 (*InterpolationFactorCallback)(f32);
    
    namespace interpolate {
        enum interpolation_transition_mode {
            itm_none = 0,
            itm_linear,
            itm_easeInQuad,
            itm_easeOutQuad,
            itm_easeInOutQuad,
            itm_easeInCubic,
            itm_easeOutCubic,
            itm_easeInOutCubic,
            itm_easeInQuart,
            itm_easeOutQuart,
            itm_easeInOutQuart,
            itm_easeInQuint,
            itm_easeOutQuint,
            itm_easeInOutQuint
        };

        // no interpolation
        f32 none (f32 normalizedInput);
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

        InterpolationFactorCallback from_enum(interpolation_transition_mode mode);
    };

    template <typename t>
    class interpolator {
        public:
			typedef t (*interpolation_function)(const t&, const t&, f32);

            interpolator (const t& initial, f32 duration, InterpolationFactorCallback method)
				: m_initial(initial), m_final(initial), m_duration(duration),
				  m_useFinishedCb(false), m_transitionMethod(method), m_isStopped(true),
				  interpolation_callback(nullptr)
			{
            }

            interpolator (const t& initial) :
                m_initial(initial), m_final(initial), m_duration(0.0f),
                m_useFinishedCb(false), m_transitionMethod(interpolate::linear),
                m_isStopped(true), interpolation_callback(nullptr)
            {
            }

            ~interpolator () { }

            void method(InterpolationFactorCallback method) { m_transitionMethod = method; }
            
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

            inline bool stopped() const { return m_isStopped; }
            
            interpolator& operator = (const t& value) {
                if(m_duration == 0.0f) {
                    set_immediate(value);
                    return *this;
                }

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
				if (interpolation_callback) return interpolation_callback(m_initial, m_final, factor);
                return m_initial + ((m_final - m_initial) * factor);
            }
        
			interpolation_function interpolation_callback;
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

    class remote_interpolator_base {
        public:
            typedef void (*finished_callback)(void*);

            remote_interpolator_base(f32 duration, InterpolationFactorCallback method) :
                m_duration(duration), m_transitionMethod(method), m_isStopped(true),
                m_finished(nullptr)
            {
            }

            inline void method(InterpolationFactorCallback method) { m_transitionMethod = method; }
            inline f32 duration () const { return m_duration; }
            inline void duration (f32 duration) { m_duration = duration; }
            inline bool stopped() const { return m_isStopped; }
            inline void then(finished_callback cb, void* data = nullptr) {
                if (m_isStopped) {
                    cb(data);
                    return;
                }
                m_finished = cb;
                m_finishedCallbackData = data;
            }

            virtual void cancel(bool setToEndValue) = 0;
            virtual void* remote_value_ptr() = 0;
            virtual bool update(f32 dt) = 0;

        protected:
            InterpolationFactorCallback m_transitionMethod;
            finished_callback m_finished;
            void* m_finishedCallbackData;
            f32 m_elapsed;
            bool m_isStopped;
            f32 m_duration;
    };

    template <typename t>
    class remote_interpolator : public remote_interpolator_base {
        public:
            typedef t (*interpolation_function)(const t&, const t&, f32);

            remote_interpolator (t* remote_value, f32 duration, InterpolationFactorCallback method) :
                remote_interpolator_base(duration, method), m_initial(*remote_value),
                m_final(*remote_value), interpolation_callback(nullptr), m_remoteValue(remote_value)
            {
            }

            remote_interpolator (t* remote_value) :
                remote_interpolator_base(0.0f, interpolate::linear), m_initial(*remote_value),
                m_final(*remote_value), m_isStopped(true), interpolation_callback(nullptr),
                m_remoteValue(remote_value)
            {
            }

            ~remote_interpolator () { }

            void set_immediate (const t& value) {
                m_initial = value;
                m_final = value;
                m_isStopped = true;
                *m_remoteValue = value;
            }

            void operator = (const t& value) {
                if(m_duration == 0.0f) {
                    set_immediate(value);
                    return;
                }

                if(!m_isStopped) m_initial = *m_remoteValue;
                m_elapsed = 0.0f;
                m_isStopped = false;
                m_final = value;
            }

            void set(const t& value) {
                if(m_duration == 0.0f) {
                    set_immediate(value);
                    return;
                }

                if(!m_isStopped) m_initial = *m_remoteValue;
                m_elapsed = 0.0f;
                m_isStopped = false;
                m_final = value;
            }

            virtual void cancel(bool setToEndValue) {
                if (setToEndValue) *m_remoteValue = m_initial = m_final;
                else m_initial = m_final = *m_remoteValue;
                m_elapsed = 0.0f;
                m_isStopped = true;
            }

            virtual void* remote_value_ptr() {
                return m_remoteValue;
            }

            virtual bool update(f32 dt) {
                if(m_isStopped) {
                    *m_remoteValue = m_final;
                    return true;
                }

                m_elapsed += dt;
                if(m_elapsed >= m_duration) {
                    m_isStopped = true;
                    m_initial = m_final;
                    *m_remoteValue = m_initial;
                    if (m_finished) m_finished(m_finishedCallbackData);
                    return true;
                }

                f32 factor = m_transitionMethod(m_elapsed / m_duration);
                if (interpolation_callback) *m_remoteValue = interpolation_callback(m_initial, m_final, factor);
                else *m_remoteValue = m_initial + ((m_final - m_initial) * factor);

                return false;
            }

            interpolation_function interpolation_callback;
        protected:
            t* m_remoteValue;
            t m_final;
            t m_initial;
    };
};
