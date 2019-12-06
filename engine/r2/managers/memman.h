#pragma once
#include <r2/config.h>
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <list>
#include <stdio.h>

namespace r2 {
	#define KBtoB(s) (s * 1024)
	#define MBtoB(s) (s * 1048576)
	#define GBtoB(s) (s * 1073741824)

	class memory_man;

	typedef u8 allocator_id;

	struct memory_block {
		size_t size;
		allocator_id used;
		memory_block* next;
	};

	class memory_allocator {
		public:
			memory_allocator(size_t max_size);
			~memory_allocator();

			void* allocate(size_t size);
			bool deallocate(void* ptr);
			void deallocate_all();

			void merge_adjacent_blocks();

			void debug(allocator_id level);
			allocator_id id() const;

			size_t size() const { return m_size; }
			size_t used() const { return m_used; }

		protected:
			friend class memory_man;
			memory_allocator();
			void deallocate_from_self(void* ptr);
			bool deallocate_called_from_other_allocator(memory_block* block, void* ptr, allocator_id otherAllocatorId);

			memory_block* find_available(size_t size);

			void* m_base;
			allocator_id m_id;
			size_t m_size;
			size_t m_used;
			size_t m_blockCount;
			size_t m_mergeCounter;
			memory_allocator* m_next;
			memory_allocator* m_last;
			memory_block* m_baseBlock;
	};

	class memory_man {
		public:
			static memory_man* get();
			~memory_man();

			static void push_current(memory_allocator* allocator);
			static void push_current(allocator_id allocator);
			static memory_allocator* pop_current();

			static memory_allocator* current();
			static memory_allocator* global();

			static void* allocate(size_t size);
			static void deallocate(void* ptr);

			static void debug();

		protected:
			friend class memory_allocator;
			memory_allocator* get_allocator_by_id(allocator_id id);
			memory_allocator* get_deepest_allocator();

			struct allocator_stack {
				allocator_stack* next;
				allocator_stack* last;
				memory_allocator* allocator;
			};

			memory_man();
			static memory_man* instance;
			size_t m_memSize;

			void* m_base;
			memory_allocator m_baseAllocator;

			allocator_stack* m_allocatorStack;
	};

	void* r2alloc(size_t sz);
	void r2free(void* ptr);

	template <typename T>
	using mvector = std::vector<T>;
	//using mvector = std::vector<T, std_allocator<T>>;

	template <typename T>
	using mlist = std::list<T>;
	//using mlist = std::list<T, std_allocator<T>>;

	template <typename K, typename T>
	using munordered_map = std::unordered_map<K, T>;
	//using munordered_map = std::unordered_map<K, T, std::hash<K>, std::equal_to<K>, std_allocator<std::pair<const K, T>>>;

	//using mstring = std::basic_string<char, std::char_traits<char>, std_allocator<char>>;
	using mstring = std::string;

	const char* format_size(size_t sz);
};

void* operator new(size_t size);
void* operator new[] (size_t size);
void operator delete(void* p);
void operator delete[] (void* ptr);