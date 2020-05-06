#include <r2/managers/memman.h>
#include <cstdlib>

namespace r2 {
	static bool use_malloc = false;
	static bool log_alloc_sizes = false;
	FILE* tfp = nullptr;

	memory_block* blockFromPtr(void *data) {
		return (memory_block*)((u8*)data - sizeof(memory_block));
	}

	void* ptrFromBlock(memory_block* block) {
		return ((u8*)block) + sizeof(memory_block);
	}

	inline size_t align(size_t n) {
		return (n + sizeof(word) - 1) & ~(sizeof(word) - 1);
	}

	inline bool checkSize(memory_block* block) {
		if (block->next) {
			size_t memDiff = size_t(block->next) - size_t(block);
			size_t shouldBe = block->size + sizeof(memory_block);
			if (memDiff != shouldBe) {
				printf("block contains some amount of data other than the amount it says it does...\n");
				return false;
			}
		}
		return true;
	}


	// replaced by active allocation tracking and storage of blocks
	// of specific (recently frequently allocated) sizes
	// this may still be useful later
	size_t init_pools(free_pool_stats* stats) {
		size_t c = 0;
		return c;
		// block count, bytes in pool, min block size, max block size
		stats[c++] = { 0, 0,     8,    15 };
		stats[c++] = { 0, 0,    16,    23 };
		stats[c++] = { 0, 0,    24,    31 };
		stats[c++] = { 0, 0,    32,    47 };
		stats[c++] = { 0, 0,    48,    63 };
		stats[c++] = { 0, 0,    64,   127 };
		stats[c++] = { 0, 0,   128,   255 };
		stats[c++] = { 0, 0,   256,   511 };
		stats[c++] = { 0, 0,   512,  1023 };
		stats[c++] = { 0, 0,  1024,  2047 };
		stats[c++] = { 0, 0,  2048,  4095 };
		stats[c++] = { 0, 0,  4096,  8191 };
		stats[c++] = { 0, 0,  8192, 16384 };
		return c;
	}


	memory_man* memory_man::instance = nullptr;

	frequency_track::frequency_track(memory_allocator* allocator) : count(0), allocs_per_second(0.0f), free_blocks(allocator) {
		last_alloc_timer.start();
	};



	memory_allocator::memory_allocator() :
		m_last(nullptr),
		m_next(nullptr),
		m_base(nullptr),
		m_size(0),
		m_used(0),
		m_blockCount(0),
		m_baseBlock(nullptr),
		m_freePoolMem(nullptr),
		m_emptyFreePools(),
		m_freePools()
	{
		m_id = 1;
		if (use_malloc) return;
		m_mergeCounter = 50;
		m_size_in_free_pools = 0;
		m_size_in_tracked_pools = 0;
		m_tracking_enabled = true;
		for (u8 i = 0;i < FREE_POOL_COUNT;i++) {
			m_freePools[i].block = nullptr;
			m_freePools[i].next = nullptr;
			m_freePoolStats[i].count = 0;
			m_freePoolStats[i].size = 0;
			m_freePoolStats[i].min_block_size = 0;
			m_freePoolStats[i].max_block_size = 0;
		}
		m_used_pool_count = init_pools(m_freePoolStats);
		m_clean_tracked_timer.start();
	}

	memory_allocator::memory_allocator(size_t max_size, size_t max_free_pool_size) : m_last(nullptr), m_next(nullptr), m_base(nullptr), m_size(max_size), m_used(0), m_blockCount(0) {
		m_last = memory_man::get()->get_deepest_allocator();
		m_last->m_next = this;
		if (use_malloc) return;
		m_base = memory_man::global()->allocate(max_size);
		m_baseBlock = (memory_block*)m_base;
		m_baseBlock->size = max_size - sizeof(memory_block);
		m_baseBlock->next = nullptr;
		m_baseBlock->used = false;
		m_used = sizeof(memory_block);
		m_blockCount++;
		m_id = m_last->m_id + 1;
		m_mergeCounter = 50;
		m_size_in_free_pools = 0;
		m_size_in_tracked_pools = 0;
		m_tracking_enabled = true;

		size_t pool_size = max_free_pool_size;
		if (pool_size == 0) pool_size = max_size * 0.1;
		if (pool_size < sizeof(free_list) * 512) pool_size = sizeof(free_list) * 512;
		size_t free_list_count = pool_size / sizeof(free_list);
		pool_size = free_list_count * sizeof(free_list);

		m_freePoolMem = memory_man::global()->allocate(pool_size);
		free_list* nodes = (free_list*)m_freePoolMem;
		m_emptyFreePools.block = nullptr;
		m_emptyFreePools.next = nodes;
		for (size_t i = 0;i < free_list_count;i++) {
			nodes[i].block = nullptr;
			nodes[i].next = i < free_list_count - 1 ? &nodes[i + 1] : nullptr;
		}

		for (u8 i = 0;i < FREE_POOL_COUNT;i++) {
			m_freePools[i].block = nullptr;
			m_freePools[i].next = nullptr;
			m_freePoolStats[i].count = 0;
			m_freePoolStats[i].size = 0;
			m_freePoolStats[i].min_block_size = 0;
			m_freePoolStats[i].max_block_size = 0;
		}

		m_used_pool_count = init_pools(m_freePoolStats);
		m_clean_tracked_timer.start();
	}

	memory_allocator::~memory_allocator() {
		if (use_malloc) return;
		if (m_last) {
			memory_man::global()->m_lock.lock();
			memory_man::global()->deallocate_from_self(m_base);
			memory_man::global()->deallocate_from_self(m_freePoolMem);
			memory_man::global()->m_lock.unlock();
			m_last->m_next = m_next;
			if (m_next) m_next->m_last = m_last;
		}
		printf("allocator destroyed 0x%X\n", (intptr_t)this);
	}

	void* memory_allocator::allocate(size_t size) {
		if (use_malloc) return malloc(size);

		m_lock.lock();
		// check if there is a free block that fits this size
		void* mem = get_free_list_node(align(size));
		if (mem) {
			m_lock.unlock();
			return mem;
		}

		memory_block* block = find_available(align(size));
		if (!block && size > m_size - m_used) {
			printf("Failed to allocate %s bytes from allocator %d, which has %s bytes available\n", format_size(size), m_id, format_size(m_size - m_used));
			m_lock.unlock();
			exit(-1);
			return nullptr;
		}

		assert(block != nullptr);

		if (!block->used) m_used += block->size;
		block->used = m_id;

		purge_unused_tracked_blocks();
		m_lock.unlock();
		return ptrFromBlock(block);
	}

	void* memory_allocator::reallocate(void* ptr, size_t size) {
		if (use_malloc) return realloc(ptr, size);

		memory_block* block = blockFromPtr(ptr);
		if (block->size == size) return ptr;
		if (block->used == m_id) {
			m_lock.lock();
			void* newPtr = reallocate_from_self(ptr, size);
			m_lock.unlock();
			if (!newPtr) {
				printf("Failed to reallocate %s bytes from allocator %d, which has %s bytes available\n", format_size(size), m_id, format_size(m_size - m_used));
				exit(-1);
				return nullptr;
			}

			return newPtr;
		}

		// this allocator didn't allocate ptr... nice
		if (block->used < m_id && m_last) {
			m_lock.lock();
			void* newPtr = m_last->reallocate_called_from_other_allocator(block, ptr, size, m_id);
			m_lock.unlock();
			if (newPtr) return newPtr;
		}

		if (block->used > m_id && m_next) {
			m_lock.lock();
			void* newPtr = m_next->reallocate_called_from_other_allocator(block, ptr, size, m_id);
			m_lock.unlock();
			if (newPtr) return newPtr;
		}

		printf("Memory leak detected. 0x%X was allocated by allocator %d, which was destroyed. Also, failed to reallocate data\n", (intptr_t)ptr, (i32)block->used);
		exit(-1);
		return nullptr;
	}

	bool memory_allocator::deallocate(void* ptr) {
		if (use_malloc) {
			free(ptr);
			return true;
		}

		m_lock.lock();
		memory_block* block = blockFromPtr(ptr);
		if (block->used == m_id) {
			deallocate_from_self(ptr);
			m_lock.unlock();
			return true;
		}

		// this allocator didn't allocate ptr... nice
		if (block->used < m_id && m_last && m_last->deallocate_called_from_other_allocator(block, ptr, m_id)) {
			m_lock.unlock();
			return true;
		}
		if (block->used > m_id && m_next && m_next->deallocate_called_from_other_allocator(block, ptr, m_id)) {
			m_lock.unlock();
			return true;
		}

		printf("Memory leak detected. 0x%X was allocated by allocator %d, which was destroyed.\n", (intptr_t)ptr, (i32)block->used);
		m_lock.unlock();
		return false;
	}

	void memory_allocator::deallocate_all() {
		if (use_malloc) return;
		m_lock.lock();
		// Note: This call doesn't deallocate anything.
		// There's no risk of deadlock due to recursion
		m_allocTrackers.clear();

		m_baseBlock = (memory_block*)m_base;
		m_baseBlock->size = m_size - sizeof(memory_block);
		m_baseBlock->next = nullptr;
		m_baseBlock->used = false;
		m_used = sizeof(memory_block);
		m_blockCount = 1;
		m_mergeCounter = 50;

		for (size_t i = 0;i < m_used_pool_count;i++) {
			free_list* node = m_freePools[i].next;
			while (node) {
				free_list* next = node->next;
				
				node->next = m_emptyFreePools.next;
				node->block = nullptr;
				m_emptyFreePools.next = node;

				node = next;
			}
		}

		for (u8 i = 0;i < FREE_POOL_COUNT;i++) {
			m_freePools[i].block = nullptr;
			m_freePools[i].next = nullptr;
			m_freePoolStats[i].count = 0;
			m_freePoolStats[i].size = 0;
			m_freePoolStats[i].min_block_size = 0;
			m_freePoolStats[i].max_block_size = 0;
		}

		memset(ptrFromBlock(m_baseBlock), 0xFC, m_baseBlock->size);
		m_lock.unlock();
	}

	void memory_allocator::debug(allocator_id level) {
		if (use_malloc) return;
		m_lock.lock();
		merge_adjacent_blocks();

		struct bucket {
			size_t size;
			size_t used_count;
			size_t unused_count;
		};
		bucket buckets[32];
		size_t other_unused = 0;
		size_t other_used = 0;
		memset(buckets, 0, sizeof(bucket) * 32);

		memory_block* b = m_baseBlock;
		while(b) {
			bool found = false;
			for(u8 i = 0;i < 32;i++) {
				if (buckets[i].size == b->size) {
					found = true;
					if (b->used) buckets[i].used_count++;
					else buckets[i].unused_count++;
					break;
				} else if(buckets[i].size == 0) {
					found = true;
					buckets[i].size = b->size;
					if (b->used) buckets[i].used_count++;
					else buckets[i].unused_count++;
					break;
				}
			}
			if (!found) {
				if (b->used) other_used++;
				else other_unused++;
			}
			b = b->next;
		}

		f32 usage = (f32(m_used) / f32(m_size)) * 100.0f;
		for(allocator_id i = 0;i < level;i++) printf("\t");
		printf("Allocator: %s / %s (%llu blocks), %0.2f%%\n", format_size(m_used), format_size(m_size), m_blockCount, usage);
		for(u8 i = 0;i < 32;i++) {
			if (buckets[i].size == 0) break;
			for(allocator_id i = 0;i < level;i++) printf("\t");
			printf("\tBlocks of size %s: used: %llu, unused: %llu\n", format_size(buckets[i].size), buckets[i].used_count, buckets[i].unused_count);
		}
		if (other_used > 0 || other_unused > 0) {
			for(allocator_id i = 0;i < level;i++) printf("\t");
			printf("\tBlocks of size not shown above: used: %llu, unused: %llu\n", other_used, other_unused);
		}

		if (m_next) m_next->debug(level + 1);
		m_lock.unlock();
	}

	allocator_id memory_allocator::id() const {
		return m_id;
	}

	size_t memory_allocator::size() {
		if (use_malloc) return UINT64_MAX;
		m_lock.lock();
		size_t ret = m_size;
		m_lock.unlock();
		return ret;
	}

	size_t memory_allocator::used() {
		if (use_malloc) return 0;
		m_lock.lock();
		size_t ret = m_used;
		m_lock.unlock();
		return ret;
	}

	void memory_allocator::slow_check() {
		memory_block* b = m_baseBlock;
		size_t i = 0;
		while(b) {
			assert(checkSize(b));
			b = b->next;
			i++;
		}
	}

	/* memory_allocator protected methods */

	void memory_allocator::enable_memory_tracking() {
		m_tracking_enabled = true;
	}

	void memory_allocator::disable_memory_tracking() {
		m_tracking_enabled = false;
	}

	void memory_allocator::merge_adjacent_blocks() {
		m_mergeCounter = 50;
		memory_block* b = m_baseBlock;
		while(b) {
			if (!b->used && b->next && !b->next->used) {
				// merge with next
				b->size += b->next->size + sizeof(memory_block);
				b->next = b->next->next;
				checkSize(b);
				m_used -= sizeof(memory_block);
				m_blockCount--;
			}
			b = b->next;
		}
	}

	void memory_allocator::deallocate_from_self(void* ptr) {
		if (use_malloc) {
			free(ptr);
			return;
		}

		memory_block* block = blockFromPtr(ptr);
		assert(block->used == m_id);

		if (add_to_free_list(block)) return;
		
		if (m_tracking_enabled) {
			auto it = m_allocTrackers.find(block->size);
			if (it != m_allocTrackers.end()) {
				frequency_track& track = it->second;
				if (track.allocs_per_second > 20) {
					m_tracking_enabled = false;
					track.free_blocks.push(block);
					m_tracking_enabled = true;
					m_size_in_tracked_pools += block->size;
					return;
				}
			}
		}

		deallocate_block(block);

		// can't merge backwards here because of the lack of memory_block::last,
		// merge forward from the beginning instead every so often

		m_mergeCounter--;
		if (m_mergeCounter == 0) merge_adjacent_blocks();

		purge_unused_tracked_blocks();
	}

	void memory_allocator::deallocate_block(memory_block* block) {
		block->used = false;
		m_used -= block->size;

		if (block->next && !block->next->used) {
			// merge with next
			block->size += block->next->size + sizeof(memory_block);
			block->next = block->next->next;
			checkSize(block);
			m_used -= sizeof(memory_block);
			m_blockCount--;
		}
	}

	bool memory_allocator::deallocate_called_from_other_allocator(memory_block* block, void* ptr, allocator_id otherAllocatorId) {
		if (use_malloc) {
			free(ptr);
			return true;
		}

		if (block->used == m_id) {
			deallocate_from_self(ptr);
			return true;
		}

		if ((block->used > m_id && otherAllocatorId > m_id) || (block->used < m_id && otherAllocatorId < m_id)) {
			// block was allocated by an allocator that was between this one and the one that called this function
			// but was deleted. The memory used by ptr was released, back to the global allocator, but this is
			// technically a memory leak
			return false;
		}

		if (block->used < m_id && m_last && m_last->deallocate_called_from_other_allocator(block, ptr, m_id)) return true;
		if (block->used > m_id && m_next && m_next->deallocate_called_from_other_allocator(block, ptr, m_id)) return true;

		return false;
	}

	void* memory_allocator::reallocate_from_self(void* ptr, size_t size) {
		if (use_malloc) return realloc(ptr, size);
		memory_block* block = blockFromPtr(ptr);
		assert(block->used == m_id);

		// should be what happens
		if (size > block->size) {
			size_t sizeDiff = size - block->size;

			// first, see if the next block can be merged into this one to accomodate the new size
			if (block->next && !block->next->used && block->next->size > sizeDiff) {
				// yup. increase the size of this block, and construct a new block after it to refer
				// to the remainder of the next block
				size_t oldNextSize = block->next->size;
				memory_block* blockAfterNext = block->next->next;

				block->size = align(size);

				memory_block* nextBlock = (memory_block*)(((u8*)ptr) + align(size));
				nextBlock->next = blockAfterNext;
				nextBlock->size = oldNextSize - sizeDiff;
				nextBlock->used = add_to_free_list(nextBlock);

				block->next = nextBlock;

				m_used += sizeDiff + sizeof(memory_block);
				if (nextBlock->used) m_used += nextBlock->size;

				return ptr;
			} else {
				// nope. allocate a new one, copy this one to it, and deallocate this one
				void* newPtr = allocate(size);
				memcpy(newPtr, ptr, block->size);
				deallocate(ptr);
				return newPtr;
			}
		} else {
			size_t sizeDiff = block->size - size;
			
			// first, see if the next block can be expanded backwards
			if (block->next && !block->next->used) {
				size_t oldNextSize = block->next->size;
				memory_block* blockAfterNext = block->next->next;

				block->size = align(size);

				memory_block* nextBlock = (memory_block*)(((u8*)ptr) + align(size));
				nextBlock->next = blockAfterNext;
				nextBlock->size = oldNextSize + sizeDiff;
				nextBlock->used = add_to_free_list(nextBlock);

				if (nextBlock->used) m_used += nextBlock->size;
				else m_used -= sizeDiff;

				block->next = nextBlock;

				return ptr;
			} else if (sizeDiff > sizeof(memory_block)) {
				// nope, but a new free block can be created in the gap left by shrinking this block

				memory_block* oldNextBlock = block->next;

				block->size = align(size);

				memory_block* nextBlock = (memory_block*)(((u8*)ptr) + align(size));
				nextBlock->next = oldNextBlock;
				nextBlock->size = sizeDiff - sizeof(memory_block);
				nextBlock->used = add_to_free_list(nextBlock);

				m_used += sizeof(memory_block);
				if (nextBlock->used) m_used += nextBlock->size;
				else m_used -= sizeDiff;

				block->next = nextBlock;

				m_blockCount++;

				return ptr;
			} else {
				// this block can't be shrunk, but that's fine. It can at least hold the data that it needs to
				return ptr;
			}
		}

		return nullptr;
	}

	void* memory_allocator::reallocate_called_from_other_allocator(memory_block* block, void* ptr, size_t size, allocator_id otherAllocatorId) {
		if (block->used == m_id) {
			return reallocate_from_self(ptr, size);
		}

		if ((block->used > m_id && otherAllocatorId > m_id) || (block->used < m_id && otherAllocatorId < m_id)) {
			// block was allocated by an allocator that was between this one and the one that called this function
			// but was deleted. The memory used by ptr was released, back to the global allocator, but this is
			// technically a memory leak
			return nullptr;
		}

		if (block->used < m_id && m_last) {
			void* newPtr = m_last->reallocate_called_from_other_allocator(block, ptr, size, m_id);
			if (newPtr) return newPtr;
		}

		if (block->used > m_id && m_next) {
			void* newPtr = m_next->reallocate_called_from_other_allocator(block, ptr, size, m_id);
			if (newPtr) return newPtr;
		}

		return nullptr;
	}

	memory_block* memory_allocator::find_available(size_t size) {
		if (m_tracking_enabled && (m_used_pool_count == 0 || size > m_freePoolStats[m_used_pool_count - 1].max_block_size)) {
			auto it = m_allocTrackers.find(size);
			if (it != m_allocTrackers.end()) {
				frequency_track& track = it->second;
				track.count++;
				f32 elapsed = track.last_alloc_timer;
				if (elapsed >= 1.0f) {
					track.last_alloc_timer.reset();
					track.last_alloc_timer.start();
					track.allocs_per_second = f32(track.count) / elapsed;
					track.count = 0;
				}

				size_t free_count = track.free_blocks.size();
				if (free_count > 0) {
					memory_block* block = *track.free_blocks[free_count - 1];
					m_tracking_enabled = false;
					track.free_blocks.remove(free_count - 1);
					m_tracking_enabled = true;
					m_size_in_tracked_pools -= block->size;
					return block;
				}
			} else {
				m_tracking_enabled = false;
				m_allocTrackers.emplace(size, this);
				m_tracking_enabled = true;
			}
		}

		if (tfp) fprintf(tfp, "%llu,", size);

		memory_block* b = m_baseBlock;
		while(b) {
			if (b->size > size + sizeof(memory_block) && !b->used) {
				u8* bData = (u8*)ptrFromBlock(b);

				if (checkSize(b)) {
					memory_block* nb = (memory_block*)(bData + size);
					nb->next = b->next;
					nb->size = b->size - size - sizeof(memory_block);
					nb->used = false;

					b->next = nb;
					b->size = size;
					b->used = false;

					checkSize(b);
					checkSize(nb);

					m_blockCount++;
					m_used += sizeof(memory_block);
					return b;
				}
			} else if(b->size == size && !b->used) {
				return b;
			}
			b = b->next;
		}

		printf("Failed to find memory for size %s\n", format_size(size));

		return nullptr;
	}

	free_list* memory_allocator::get_empty_free_list_node() {
		if (!m_emptyFreePools.next) return nullptr;

		free_list* next = m_emptyFreePools.next;
		m_emptyFreePools.next = next->next;

		return next;
	}

	bool memory_allocator::add_to_free_list(memory_block* block) {
		if ((m_size_in_free_pools + block->size) > (m_size / 4)) {
			return false;
		}

		u8 pidx = 0;
		for (u8 i = 0;i < m_used_pool_count;i++) {
			if (block->size >= m_freePoolStats[i].min_block_size && block->size < m_freePoolStats[i].max_block_size) break;
			pidx++;
		}

		if (pidx < m_used_pool_count) {
			// this block can fit in a free pool
			free_list* node = get_empty_free_list_node();
			if (node) {
				node->block = block;
				node->next = m_freePools[pidx].next;
				m_freePools[pidx].next = node;
				m_freePoolStats[pidx].count++;
				m_freePoolStats[pidx].size += block->size;
				m_size_in_free_pools += block->size;

				return true;
			}
		}

		return false;
	}

	void* memory_allocator::get_free_list_node(size_t size) {
		u8 pidx = 0;
		for (u8 i = 0;i < m_used_pool_count;i++) {
			if (size <= m_freePoolStats[i].min_block_size) break;
			pidx++;
		}
		if (pidx == m_used_pool_count) return nullptr;

		free_list* pool = m_freePools[pidx].next;
		if (!pool) return nullptr;

		// remove from free pool
		m_freePools[pidx].next = pool->next;

		// store represented memory block
		memory_block* block = pool->block;

		// insert into empty node list
		pool->block = nullptr;
		pool->next = m_emptyFreePools.next;
		m_emptyFreePools.next = pool;

		m_size_in_free_pools -= block->size;
		m_freePoolStats[pidx].count--;
		m_freePoolStats[pidx].size -= block->size;

		return ptrFromBlock(block);
	}

	memory_block* memory_allocator::block(size_t idx) {
		size_t i = 0;
		memory_block* b = m_baseBlock;
		while(b && i <= idx) {
			b = b->next;
			i++;
		}
		return b;
	}

	void memory_allocator::purge_unused_tracked_blocks() {
		if (m_clean_tracked_timer.elapsed() < 10.0f) return;
		m_clean_tracked_timer.reset();
		m_clean_tracked_timer.start();


		//printf("purging unused blocks: %s -> ", format_size(m_size_in_tracked_pools));
		for (auto& i = m_allocTrackers.begin();i != m_allocTrackers.end();++i) {
			frequency_track& track = i->second;
			if (track.free_blocks.size() > 0 && track.last_alloc_timer.elapsed() > 10.0f) {
				track.free_blocks.for_each([this](memory_block** block) {
					this->deallocate_block(*block);
					this->m_size_in_tracked_pools -= (*block)->size;
					return true;
				});
				track.free_blocks.clear();
				track.allocs_per_second = 0;
				track.count = 0;
			}
		}
		//printf("%s\n", format_size(m_size_in_tracked_pools));
	}



	memory_man* memory_man::get() {
		if (!instance) {
			instance = (memory_man*)malloc(sizeof(memory_man));
			new (instance) memory_man();
		}
		return instance;
	}

	memory_man::memory_man() {
		if (log_alloc_sizes) {
			tfp = fopen("memstat.txt", "w");
			if (!tfp) printf("Failed to create memstat.txt\n");
		}

		FILE* fp = fopen("mem.ini", "rb");
		if (!fp) {
			printf("No mem.ini found. Creating one, maximum memory usage by this application is now 32 MB. Take that.\n");
			fp = fopen("mem.ini", "w");
			if (fp) {
				m_memSize = MBtoB(32);
				fprintf(fp, "%llu", m_memSize);
				fclose(fp);
			} else {
				printf("Failed to create mem.ini...\n");
			}
		} else {
			fseek(fp, 0, SEEK_END);
			size_t fsz = ftell(fp);
			fseek(fp, 0, SEEK_SET);
			char msz[64] = { 0 };
			if (fsz > 0 && fsz < 64) {
				if (fread(msz, 1, fsz, fp) == fsz) {
					m_memSize = strtol(msz, nullptr, 10);
					if (m_memSize == 0 || errno == ERANGE) {
						printf("Invalid memory size specified in mem.ini. bye\n");
						fclose(fp);
						exit(-1);
						return;
					}
				} else {
					printf("Failed to read memory size from mem.ini. bye\n");
					fclose(fp);
					exit(-1);
					return;
				}
			} else {
				printf("mem.ini is either empty or too big. It must be between 0 and 64 bytes. bye\n");
				fclose(fp);
				exit(-1);
				return;
			}
			fclose(fp);
		}

		if (use_malloc) m_memSize = 1024;

		m_base = malloc(m_memSize);
		m_baseAllocator.m_base = m_base;
		m_baseAllocator.m_size = m_memSize;
		m_baseAllocator.m_baseBlock = (memory_block*)m_base;
		m_baseAllocator.m_baseBlock->size = m_memSize - sizeof(memory_block);
		m_baseAllocator.m_baseBlock->next = nullptr;
		m_baseAllocator.m_baseBlock->used = false;
		m_baseAllocator.m_used = sizeof(memory_block);
		m_baseAllocator.m_blockCount++;

		size_t pool_size = m_memSize * 0.1;
		if (pool_size < sizeof(free_list) * 512) pool_size = sizeof(free_list) * 512;
		size_t free_list_count = pool_size / sizeof(free_list);
		pool_size = free_list_count * sizeof(free_list);

		m_baseAllocator.m_freePoolMem = malloc(pool_size);
		free_list* nodes = (free_list*)m_baseAllocator.m_freePoolMem;
		m_baseAllocator.m_emptyFreePools.block = nullptr;
		m_baseAllocator.m_emptyFreePools.next = nodes;
		for (size_t i = 0;i < free_list_count;i++) {
			nodes[i].block = nullptr;
			nodes[i].next = i < free_list_count - 1 ? &nodes[i + 1] : nullptr;
		}

		m_allocatorStack = (allocator_stack*)m_baseAllocator.allocate(sizeof(allocator_stack));
		m_allocatorStack->last = nullptr;
		m_allocatorStack->next = nullptr;
		m_allocatorStack->allocator = &m_baseAllocator;
	}

	memory_man::~memory_man() {
		if (tfp) fclose(tfp);
		free(m_base);
		free(m_baseAllocator.m_freePoolMem);
	}

	memory_allocator* memory_man::get_allocator_by_id(allocator_id id) {
		if (use_malloc) return &instance->m_baseAllocator;
		m_lock.lock();
		memory_allocator* s = &m_baseAllocator;
		while(s) {
			if (s->m_id == id) {
				m_lock.unlock();
				return s;
			}
			s = s->m_next;
		}

		m_lock.unlock();
		return nullptr;
	}

	void memory_man::push_current(memory_allocator* allocator) {
		if (use_malloc) return;
		// remains locked until the corresponding pop_current is called
		instance->m_lock.lock();
		allocator_stack* s = (allocator_stack*)instance->m_baseAllocator.allocate(sizeof(allocator_stack));
		s->last = nullptr;
		s->next = instance->m_allocatorStack;
		s->allocator = allocator;
		instance->m_allocatorStack->last = s;
		instance->m_allocatorStack = s;
		// instance->m_lock.unlock();
	}

	void memory_man::push_current(allocator_id allocatorId) {
		if (use_malloc) return;
		memory_allocator* allocator = instance->get_allocator_by_id(allocatorId);
		if (!allocator) {
			printf("Allocator with ID %d was destroyed or never existed. Using global allocator instead\n", allocatorId);
			memory_man::push_current(memory_man::global());
		} else memory_man::push_current(allocator);
	}

	memory_allocator* memory_man::pop_current() {
		if (use_malloc) return &instance->m_baseAllocator;
		// instance->m_lock.lock();
		allocator_stack* cur = instance->m_allocatorStack;
		memory_allocator* allocator = cur->allocator;

		instance->m_allocatorStack = cur->next;
		instance->m_allocatorStack->last = nullptr;
		instance->m_baseAllocator.m_lock.lock();
		instance->m_baseAllocator.deallocate_from_self(cur);
		instance->m_baseAllocator.m_lock.unlock();

		// lock was locked at the corresponding push_current call
		instance->m_lock.unlock();
		return allocator;
	}

	memory_allocator* memory_man::get_deepest_allocator() {
		if (use_malloc) return &instance->m_baseAllocator;
		m_lock.lock();
		memory_allocator* s = &m_baseAllocator;
		while(s) {
			if (!s->m_next) {
				m_lock.unlock();
				return s;
			}
			s = s->m_next;
		}
		m_lock.unlock();
		return &m_baseAllocator;
	}

	memory_allocator* memory_man::current() {
		if (use_malloc) return &instance->m_baseAllocator;
		instance->m_lock.lock();
		memory_allocator* allocator = instance->m_allocatorStack->allocator;
		instance->m_lock.unlock();
		return allocator;
	}

	memory_allocator* memory_man::global() {
		if (use_malloc) return &instance->m_baseAllocator;
		instance->m_lock.lock();
		memory_allocator* allocator = &instance->m_baseAllocator;
		instance->m_lock.unlock();
		return allocator;
	}

	void* memory_man::allocate(size_t size) {
		if (use_malloc) return malloc(size);
		instance->m_lock.lock();
		void* result = instance->m_allocatorStack->allocator->allocate(size);
		instance->m_lock.unlock();
		return result;
	}

	void* memory_man::reallocate(void* ptr, size_t size) {
		if (use_malloc) return realloc(ptr, size);
		instance->m_lock.lock();
		void* result = instance->m_allocatorStack->allocator->reallocate(ptr, size);
		instance->m_lock.unlock();
		return result;
	}

	void memory_man::deallocate(void* ptr) {
		if (use_malloc) return free(ptr);
		instance->m_lock.lock();
		instance->m_allocatorStack->allocator->deallocate(ptr);
		instance->m_lock.unlock();
	}

	void memory_man::debug() {
		if (use_malloc) return;
		printf("\n\n\nmemory_man::debug()\n");
		instance->m_lock.lock();
		instance->m_baseAllocator.debug(0);
		memory_allocator* allocator = &instance->m_baseAllocator;
		while(allocator) {
			char num[4] = { 0 };
			snprintf(num, 4, "%d", allocator->m_id);
			string fn = string("mem_s") + num + ".dat";
			FILE* fp = fopen(fn.c_str(), "wb");
			if (fp) {
				if(fwrite(allocator->m_base, allocator->m_size, 1, fp) != 1) {
					printf("Failed to save memory\n");
				}
				fclose(fp);
			}
			allocator = allocator->m_next;
		}
		instance->m_lock.unlock();
	}
	
	void memory_man::lock() {
		if (use_malloc) return;
		instance->m_lock.lock();
	}

	void memory_man::unlock() {
		if (use_malloc) return;
		instance->m_lock.unlock();
	}

	void* r2alloc(size_t sz) {
		return memory_man::get()->allocate(sz);
	}

	void* r2calloc(size_t count, size_t size) {
		void* d = r2alloc(count * size);
		memset(d, 0, count * size);
		return d;
	}

	void* r2realloc(void* ptr, size_t sz) {
		if (!ptr) {
			printf("Something tried to reasize a null pointer... Returning allocated memory with desired size\n");
			return r2alloc(sz);
		}

		return memory_man::get()->reallocate(ptr, sz);
	}

	void r2free(void* ptr) {
		if (!ptr) {
			printf("Something tried to delete a null pointer...\n");
			return;
		}
		memory_man::get()->deallocate(ptr);
	}

	const char* format_size(size_t sz) {
		f64 mult = 1.0;
		const char* unit = "B";
		if (sz > KBtoB(1)) { mult = 1.0 / 1024.0; unit = "KB"; }
		if (sz > MBtoB(1)) { mult = 1.0 / (1024.0 * 1024.0); unit = "MB"; }
		if (sz > GBtoB(1)) { mult = 1.0 / (1024.0 * 1024.0 * 1024.0); unit = "GB"; }

		static u8 idx = 0;
		static char szStr[16][16] = { 0 };

		snprintf(szStr[idx], 16, "%0.2f %s", f64(sz) * mult, unit);
		char* ret = szStr[idx];
		idx++;
		if (idx >= 15) idx = 0;

		return ret;
	}
};

void* operator new(size_t size) { return r2::r2alloc(size); }
void* operator new[] (size_t size) throw (std::bad_alloc) { return operator new(size); }
void operator delete(void* p) { r2::r2free(p); }
void operator delete[] (void* ptr) throw() { operator delete(ptr); }