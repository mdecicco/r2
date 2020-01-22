#pragma once
#include <r2/managers/memman.h>

namespace r2 {
	class untyped_dynamic_pod_array {
		public:
			untyped_dynamic_pod_array(size_t elementSize) : m_elementSize(elementSize), m_count(0), m_capacity(16), m_data(new u8[elementSize * 16]) {
			}

			untyped_dynamic_pod_array(size_t elementSize, size_t count) : m_elementSize(elementSize), m_count(count), m_capacity(count), m_data(new u8[elementSize * count]) {
			}

			~untyped_dynamic_pod_array() {
				delete [] m_data;
				m_data = nullptr;
			}

			untyped_dynamic_pod_array(const untyped_dynamic_pod_array&) = delete;
			void operator=(const untyped_dynamic_pod_array&) = delete;

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
			T* construct(construction_args ... args) {
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
					if (m_count < m_capacity / 2 && m_capacity > 16) {
						m_capacity /= 2;
						m_data = (u8*)r2realloc(m_data, m_capacity * m_elementSize);
					}
					return;
				}

				u8* src = m_data + ((index + 1) * m_elementSize);
				u8* dst = m_data + (index * m_elementSize);
				size_t sz = m_elementSize * (m_count - index);
				memcpy(dst, src, sz);

				if (m_count < m_capacity / 2 && m_capacity > 16) {
					m_capacity /= 2;
					m_data = (u8*)r2realloc(m_data, m_capacity * m_elementSize);
				}
			}

			template <typename T, typename F>
			size_t for_each(F&& callback) {
				assert(sizeof(T) == m_elementSize);
				if (m_count == 0) return 0;
				size_t count = 0;
				for (size_t i = 0;i < m_count;i++) {
					if(!callback(((T*)(m_data + (i * m_elementSize))))) return count + 1;
					count++;
				}
				return count;
			}

			template <typename T, typename F>
			size_t reverse_for_each(F&& callback) {
				assert(sizeof(T) == m_elementSize);
				if (m_count == 0) return 0;
				size_t count = 0;
				for (size_t i = m_count;i > 0;i--) {
					if(!callback(((T*)(m_data + ((i - 1) * m_elementSize))))) return count + 1;
					count++;
				}
				return count;
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

			void clear() {
				m_count = 0;
				if (m_capacity != 16) {
					m_capacity = 16;
					r2realloc(m_data, m_elementSize * 16);
				}
			}

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

			untyped_associative_pod_array(const untyped_associative_pod_array&) = delete;
			void operator=(const untyped_associative_pod_array&) = delete;

			void* set(const K& key) {
				m_offsets[key] = m_values.size();
				return m_values.push();
			}

			template <typename T, typename ... construction_args>
			T* construct(const K& key, construction_args ... args) {
				m_offsets[key] = m_values.size();
				return m_values.construct<T>(args...);
			}

			void* get(const K& key) {
				return m_values.at(m_offsets[key]);
			}

			template <typename T>
			T* get(const K& key) {
				return m_values.at<T>(m_offsets[key]);
			}

			bool has(const K& key) {
				return m_offsets.count(key) > 0;
			}

			void remove(const K& key) {
				size_t idx = m_offsets[key];
				m_offsets.erase(key);

				m_values.remove(idx);
				for (auto& offset : m_offsets) {
					if (offset.second >= idx) offset.second--;
				}
			}

			template <typename T, typename F>
			size_t for_each(F&& callback) {
				using F_type = typename std::decay<F>::type;
				return m_values.for_each<T, F>(std::forward<F_type>(callback));
			}

			template <typename T, typename F>
			size_t reverse_for_each(F&& callback) {
				using F_type = typename std::decay<F>::type;
				return m_values.reverse_for_each<T, F>(std::forward<F_type>(callback));
			}

			void clear() {
				m_values.clear();
				m_offsets.clear();
			}

			mvector<K> keys() const {
				mvector<K> keys;
				for(auto& key : m_offsets) {
					keys.push_back(key.first);
				}
				return keys;
			}

		protected:
			untyped_dynamic_pod_array m_values;
			munordered_map<K, size_t> m_offsets;
	};
	
	template <typename T>
	class dynamic_pod_array {
		public:
			dynamic_pod_array() : m_count(0), m_capacity(16), m_data(new T[16]) {
			}

			dynamic_pod_array(size_t count) : m_count(count), m_capacity(count), m_data(new T[count]) {
			}

			~dynamic_pod_array() {
				delete [] m_data;
				m_data = nullptr;
			}

			dynamic_pod_array(const dynamic_pod_array&) = delete;
			void operator=(const dynamic_pod_array&) = delete;

			void push(const T& value) {
				if (m_count == m_capacity) {
					m_data = (T*)r2realloc(m_data, m_capacity * 2 * sizeof(T));
					m_capacity *= 2;
				}

				memcpy(m_data + m_count, &value, sizeof(T));
				m_count++;
			}

			template <typename ... construction_args>
			void construct(construction_args ... args) {
				if (m_count == m_capacity) {
					m_data = (T*)r2realloc(m_data, m_capacity * 2 * sizeof(T));
					m_capacity *= 2;
				}

				new (m_data + m_count) T(args...);
				m_count++;
			}

			void remove(size_t index) {
				m_count--;
				if (index == m_count) {
					if (m_count < m_capacity / 2 && m_capacity > 16) {
						m_capacity /= 2;
						m_data = (T*)r2realloc(m_data, m_capacity * sizeof(T));
					}
					return;
				}

				T* src = m_data + (index + 1);
				T* dst = m_data + index;
				size_t sz = sizeof(T) * (m_count - index);
				memcpy(dst, src, sz);

				if (m_count < m_capacity / 2 && m_capacity > 16) {
					m_capacity /= 2;
					m_data = (T*)r2realloc(m_data, m_capacity * sizeof(T));
				}
			}

			template <typename F>
			size_t for_each(F&& callback) {
				if (m_count == 0) return 0;
				size_t count = 0;
				for (size_t i = 0;i < m_count;i++) {
					if (!callback(&m_data[i])) return count + 1;
					count++;
				}
				return count;
			}

			template <typename F>
			size_t reverse_for_each(F&& callback) {
				if (m_count == 0) return 0;
				size_t count = 0;
				for (size_t i = m_count;i > 0;i--) {
					if (!callback(&m_data[i - 1])) return count + 1;
					count++;
				}
				return count;
			}

			inline T* at(size_t index) { return &m_data[index]; }
			inline T* operator[](size_t index) { return &m_data[index]; }
			
			void clear() {
				m_count = 0;
				if (m_capacity != 16) {
					m_capacity = 16;
					r2realloc(m_data, sizeof(T) * 16);
				}
			}

			inline size_t size() const { return m_count; }

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
			associative_pod_array(const associative_pod_array&) = delete;
			void operator=(const associative_pod_array&) = delete;

			void set(const K& key, const T& value) {
				m_offsets[key] = m_values.size();
				m_values.push(value);
			}

			template <typename ... construction_args>
			void construct(const K& key, construction_args ... args) {
				m_offsets[key] = m_values.size();
				m_values.construct(args...);
			}

			T* get(const K& key) {
				return m_values.at(m_offsets[key]);
			}

			bool has(const K& key) {
				return m_offsets.count(key) > 0;
			}

			void remove(const K& key) {
				size_t idx = m_offsets[key];
				m_offsets.erase(key);

				m_values.remove(idx);
				for (auto& offset : m_offsets) {
					if (offset.second >= idx) offset.second--;
				}
			}

			template <typename F>
			size_t for_each(F&& callback) {
				using F_type = typename std::decay<F>::type;
				return m_values.for_each<F>(std::forward<F_type>(callback));
			}

			template <typename F>
			size_t reverse_for_each(F&& callback) {
				using F_type = typename std::decay<F>::type;
				return m_values.reverse_for_each<F>(std::forward<F_type>(callback));
			}

			void clear() {
				m_values.clear();
				m_offsets.clear();
			}

			mvector<K> keys() const {
				mvector<K> keys;
				for(auto& key : m_offsets) {
					keys.push_back(key.first);
				}
				return keys;
			}

		protected:
			dynamic_pod_array<T> m_values;
			munordered_map<K, size_t> m_offsets;
	};
};
