#pragma once
#include <r2/config.h>
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <list>
#include <stdio.h>
#include <r2/utilities/robin_hood.h>
#include <r2/utilities/timer.h>
#include <r2/utilities/dynamic_array.hpp>

namespace r2 {
	#define KBtoB(s) (s * 1024)
	#define MBtoB(s) (s * 1048576)
	#define GBtoB(s) (s * 1073741824)

	#define FREE_POOL_COUNT 24

	class memory_man;
	class memory_allocator;

	typedef u8 allocator_id;

	struct memory_block {
		size_t size;
		allocator_id used;
		memory_block* next;
	};

	struct free_list {
		memory_block* block;
		free_list* next;
	};

	struct free_pool_stats {
		size_t count;
		size_t size;
		size_t min_block_size;
		size_t max_block_size;
	};

	struct frequency_track {
		frequency_track(memory_allocator* allocator);

		u32 count;
		f32 allocs_per_second;
		timer last_alloc_timer;
		dynamic_pod_array<memory_block*> free_blocks;
	};

	class memory_allocator {
		public:
			memory_allocator(size_t max_size, size_t max_free_pool_size = 0);
			~memory_allocator();

			void* allocate(size_t size);
			void* reallocate(void* ptr, size_t size);
			bool deallocate(void* ptr);
			void deallocate_all();
			void enable_memory_tracking();
			void disable_memory_tracking();

			void merge_adjacent_blocks();

			void debug(allocator_id level);
			allocator_id id() const;

			size_t size() const { return m_size; }
			size_t used() const { return m_used; }

			void slow_check();

		protected:
			friend class memory_man;
			memory_allocator();
			void deallocate_from_self(void* ptr);
			void deallocate_block(memory_block* block);
			bool deallocate_called_from_other_allocator(memory_block* block, void* ptr, allocator_id otherAllocatorId);
			void* reallocate_from_self(void* ptr, size_t size);
			void* reallocate_called_from_other_allocator(memory_block* block, void* ptr, size_t size, allocator_id otherAllocatorId);

			memory_block* find_available(size_t size);
			free_list* get_empty_free_list_node();
			bool add_to_free_list(memory_block* block);
			void* get_free_list_node(size_t size);
			memory_block* block(size_t idx);
			void purge_unused_tracked_blocks();


			void* m_base;
			allocator_id m_id;
			size_t m_size;
			size_t m_used;
			size_t m_blockCount;
			size_t m_mergeCounter;
			memory_allocator* m_next;
			memory_allocator* m_last;
			memory_block* m_baseBlock;
			void* m_freePoolMem;
			free_list m_emptyFreePools;
			free_list m_freePools[FREE_POOL_COUNT];
			free_pool_stats m_freePoolStats[FREE_POOL_COUNT];
			size_t m_used_pool_count;
			size_t m_size_in_free_pools;
			size_t m_size_in_tracked_pools;
			bool m_tracking_enabled;
			timer m_clean_tracked_timer;
			robin_hood::unordered_map<size_t, frequency_track> m_allocTrackers;
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
			static void* reallocate(void* ptr, size_t size);
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
	void* r2calloc(size_t count, size_t size);
	void* r2realloc(void* ptr, size_t sz);
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