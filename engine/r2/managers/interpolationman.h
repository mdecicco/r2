#pragma once
#include <r2/utilities/interpolator.hpp>
#include <r2/utilities/timer.h>
#include <r2/managers/memman.h>

namespace r2 {
	class interpolation_man {
		public:
			interpolation_man();
			~interpolation_man();

			template <typename t>
			remote_interpolator<t>* animate(t* remote_value, const t& final_value, f32 duration, InterpolationFactorCallback transition = interpolate::linear) {
				if (!transition) {
					*remote_value = final_value;

					auto it = m_interpolatorMap.find(remote_value);
					if (it != m_interpolatorMap.end()) cancel(it->second, false);
					return nullptr;
				}

				if (m_free_list.next) {
					ilist* node = m_free_list.next;
					m_free_list.next = node->next;
					node->next = m_interpolators.next;
					m_interpolators.next = node;
					remote_interpolator<t>* i = new remote_interpolator<t>(remote_value, duration, transition);
					i->set(final_value);
					node->interp = i;
					m_interpolatorMap[remote_value] = i;
					return i;
				}

				r2Warn("No free remote interpolators... Setting remote value to final value immediately");
				*remote_value = final_value;
				return nullptr;
			}

			void cancel(remote_interpolator_base* interpolation, bool setToEndValue = false);
			void clear();
			void update();

		protected:
			struct ilist {
				remote_interpolator_base* interp;
				ilist* next;
			};

			munordered_map<void*, remote_interpolator_base*> m_interpolatorMap;

			ilist* m_list_mem_base;
			ilist m_free_list;
			ilist m_interpolators;
			timer m_timer;
			f32 m_last_time;
	};
};
