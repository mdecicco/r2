#pragma once

#include <r2/managers/memman.h>
#include <stddef.h>

namespace r2 {
    struct gpu_buffer_segment {
		gpu_buffer_segment() : begin(0), end(0), memBegin(0), memEnd(0) { }
		~gpu_buffer_segment() { }

        size_t begin;
        size_t end;
        size_t memBegin;
        size_t memEnd;

        size_t size() const { return end - begin; }
        size_t memsize() const { return memEnd - memBegin; }
        bool is_valid() const { return memsize() != 0; }
    };
    
    class gpu_buffer {
        public:
			typedef struct { size_t begin, end; } changed_buffer_segment;

			gpu_buffer(size_t max_size);
            virtual ~gpu_buffer();

			virtual void* data() const = 0;

            size_t id() const;

			bool has_updates() const;
			const mlist<changed_buffer_segment>& updates() const;
			void clear_updates();

			size_t used_size() const;
			size_t unused_size() const;
			size_t max_size() const;

			void appended(size_t begin, size_t end);
			void updated(size_t begin, size_t end);

        protected:
            size_t m_id;
			size_t m_size;
			size_t m_used;
			mlist<changed_buffer_segment> m_updates;
    };

	class render_driver;

	class buffer_pool {
		public:
			buffer_pool();
			~buffer_pool();

			void sync_buffers(render_driver* driver);
			void free_buffers(render_driver* driver);

			template<typename T, typename ... construction_args>
			T* find_buffer(size_t bytesNeeded, construction_args ... args) {
				for (auto buf : m_buffers) {
					if (buf->unused_size() >= bytesNeeded) return (T*)buf;
				}

				// no buffer available with enough size
				T* buf = new T(args...);
				m_buffers.push_back(buf);
				return (T*)buf;
			}

		//protected:
			mvector<gpu_buffer*> m_buffers;
	}; 
}
