#pragma once
#include <r2/managers/memman.h>

namespace r2 {
	class untyped_dynamic_pod_array {
		public:
			untyped_dynamic_pod_array(size_t elementSize) : m_elementSize(elementSize), m_count(0), m_capacity(16), m_data(new u8[elementSize * 16]) {
			}
			~untyped_dynamic_pod_array() {
				delete [] m_data;
				m_data = nullptr;
			}

			void* push() {
				if (m_count == m_capacity) {
					m_data = (u8*)r2realloc(m_data, m_capacity * 2 * m_elementSize);
					m_capacity *= 2;
				}

				u8* ptr = m_data + (m_count * m_elementSize);
				m_count++;

				return ptr;
			}

			template <typename T, typename ... construction_args>
			T* push(construction_args ... args) {
				assert(sizeof(T) == m_elementSize);

				if (m_count == m_capacity) {
					m_data = (u8*)r2realloc(m_data, m_capacity * 2 * m_elementSize);
					m_capacity *= 2;
				}

				u8* ptr = m_data + (m_count * m_elementSize);
				m_count++;

				return new (ptr) T(args...);
			}

			void remove(size_t index) {
				m_count--;
				if (index == m_count) {
					if (m_count < m_capacity / 2) {
						m_capacity /= 2;
						m_data = (u8*)r2realloc(m_data, m_capacity * m_elementSize);
					}
					return;
				}

				u8* src = m_data + ((index + 1) * m_elementSize);
				u8* dst = m_data + (index * m_elementSize);
				size_t sz = m_elementSize * (m_count - index);
				memcpy(dst, src, sz);

				if (m_count < m_capacity / 2) {
					m_capacity /= 2;
					m_data = (u8*)r2realloc(m_data, m_capacity * m_elementSize);
				}
			}

			template <typename T>
			void for_each(void (*callback)(T*, size_t, bool&)) {
				assert(sizeof(T) == m_elementSize);
				if (m_count == 0) return;
				bool brk = false;
				for (size_t i = 0;i < m_count && !brk;i++) callback((T*)(m_data + (i * m_elementSize)), i, brk);
			}

			template <typename T>
			void for_each(void (*callback)(T*)) {
				assert(sizeof(T) == m_elementSize);
				if (m_count == 0) return;
				for (size_t i = 0;i < m_count;i++) callback(((T*)(m_data + (i * m_elementSize))));
			}

			inline void* at(size_t index) {
				assert(index < m_count);
				return m_data + (index * m_elementSize);
			}

			inline void* operator[](size_t index) {
				assert(index < m_count);
				return m_data + (index * m_elementSize);
			}

			template <typename T>
			inline T* at(size_t index) {
				assert(sizeof(T) == m_elementSize);
				assert(index < m_count);
				return (T*)(m_data + (index * m_elementSize));
			}

			template <typename T>
			inline T* operator[](size_t index) {
				assert(sizeof(T) == m_elementSize);
				assert(index < m_count);
				return (T*)(m_data + (index * m_elementSize));
			}

			inline size_t size() { return m_count; }

		protected:
			u8* m_data;
			size_t m_count;
			size_t m_capacity;
			size_t m_elementSize;
	};

	template <typename K>
	class untyped_associative_pod_array {
		public:
			untyped_associative_pod_array(size_t elementSize) : m_values(elementSize) {
			}
			~untyped_associative_pod_array() {
			}

			void* set(const K& key) {
				m_offsets[key] = m_values.size();
				return m_values.push();
			}

			template <typename T, typename ... construction_args>
			T* set(const K& key, construction_args ... args) {
				m_offsets[key] = m_values.size();
				return m_values.push<T>(args...);
			}

			void* get(const K& key) {
				return m_values.at(m_offsets[key]);
			}

			template <typename T>
			T* get(const K& key) {
				return m_values.at<T>(m_offsets[key]);
			}

			void remove(const K& key) {
				size_t idx = m_offsets[key];
				m_offsets.erase(key);

				m_values.remove(idx);
				for (auto offset : m_offsets) {
					if (offset.second >= idx) m_offsets[offset.first]--;
				}
			}

			template <typename T>
			void for_each(void (*callback)(T*, size_t, bool&)) { m_values.for_each<T>(callback); }

			template <typename T>
			void for_each(void (*callback)(T*)) { m_values.for_each<T>(callback); }

		protected:
			untyped_dynamic_pod_array m_values;
			munordered_map<K, size_t> m_offsets;
	};
	
	template <typename T>
	class dynamic_pod_array {
		public:
			dynamic_pod_array() : m_count(0), m_capacity(16), m_data(new T[16]) {
			}
			~dynamic_pod_array() {
				delete [] m_data;
				m_data = nullptr;
			}

			void push(const T& value) {
				if (m_count == m_capacity) {
					m_data = r2realloc(m_data, m_capacity * 2 * sizeof(T));
					m_capacity *= 2;
				}

				memcpy(m_data + m_count, &value, sizeof(T));
				m_count++;
			}

			template <typename ... construction_args>
			void push(construction_args ... args) {
				if (m_count == m_capacity) {
					m_data = r2realloc(m_data, m_capacity * 2 * sizeof(T));
					m_capacity *= 2;
				}

				new (m_data + m_count) T(args...);
				m_count++;
			}

			void remove(size_t index) {
				m_count--;

				if (m_count < m_capacity / 2) {
					m_data = r2realloc(m_data, (m_capacity / 2) * sizeof(T));
					m_capacity /= 2;
				}

				if (index == m_count) return;

				memcpy(m_data + index, m_data + (index + 1), sizeof(T) * (m_count - (index + 1)));
			}

			void for_each(void (*callback)(T*, size_t, bool&)) {
				if (m_count == 0) return;
				bool brk = false;
				for (size_t i = 0;i < m_count && !brk;i++) callback(&m_data[i], i, brk);
			}

			void for_each(void (*callback)(T*)) {
				if (m_count == 0) return;
				for (size_t i = 0;i < m_count;i++) callback(&m_data[i]);
			}

			inline T* at(size_t index) { return &m_data[index]; }
			inline T* operator[](size_t index) { return &m_data[index]; }
			

		protected:
			T* m_data;
			size_t m_count;
			size_t m_capacity;
	};

	template <typename K, typename T>
	class associative_pod_array {
		public:
			associative_pod_array() {
			}
			~associative_pod_array() {
			}

			void set(const K& key, const T& value) {
				m_offsets[key] = m_values.size();
				m_values.push(value);
			}

			template <typename ... construction_args>
			void set(const K& key, construction_args ... args) {
				m_offsets[key] = m_values.size();
				m_values.push(args...);
			}

			T* get(const K& key) {
				return m_values.at(m_offsets[key]);
			}

			void remove(const K& key) {
				size_t idx = m_offsets[key];
				m_offsets.erase(key);

				m_values.remove(idx);
				for (auto offset : m_offsets) {
					if (offset.second >= idx) offset.second--;
				}
			}

			void for_each(void (*callback)(T*, size_t, bool&)) { m_values.for_each(callback); }
			void for_each(void (*callback)(T*)) { m_values.for_each(callback); }

		protected:
			dynamic_pod_array<T> m_values;
			munordered_map<K, size_t> m_offsets;
	};
};
