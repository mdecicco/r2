#include <r2/engine.h>
#include <cstdlib>

namespace r2 {
	static bool use_malloc = true;

	memory_block* blockFromPtr(void *data) {
		return (memory_block*)((u8*)data - sizeof(memory_block));
	}

	void* ptrFromBlock(memory_block* block) {
		return ((u8*)block) + sizeof(memory_block);
	}

	inline size_t align(size_t n) {
		return (n + sizeof(word) - 1) & ~(sizeof(word) - 1);
	}

	bool checkSize(memory_block* block) {
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


	memory_man* memory_man::instance = nullptr;

	memory_allocator::memory_allocator() : m_last(nullptr), m_next(nullptr), m_base(nullptr), m_size(0), m_used(0), m_blockCount(0), m_baseBlock(nullptr) {
		m_id = 1;
		m_mergeCounter = 50;
	}

	memory_allocator::memory_allocator(size_t max_size) : m_last(nullptr), m_next(nullptr), m_base(nullptr), m_size(max_size), m_used(0), m_blockCount(0) {
		m_last = r2engine::memory()->get_deepest_allocator();
		m_last->m_next = this;
		m_base = memory_man::global()->allocate(max_size);
		m_baseBlock = (memory_block*)m_base;
		m_baseBlock->size = max_size - sizeof(memory_block);
		m_baseBlock->next = nullptr;
		m_baseBlock->used = false;
		m_used = sizeof(memory_block);
		m_blockCount++;
		m_id = m_last->m_id + 1;
		m_mergeCounter = 50;
	}

	memory_allocator::~memory_allocator() {
		if (m_last) {
			memory_man::global()->deallocate_from_self(m_base);
			m_last->m_next = m_next;
			if (m_next) m_next->m_last = m_last;
		}
		printf("allocator destroyed 0x%X\n", (void*)this);
	}

	void* memory_allocator::allocate(size_t size) {
		if (use_malloc) return malloc(size);
		if (size > m_size - m_used) {
			if (m_id == 1) printf("Failed to allocate %s bytes from allocator %d, which has %s bytes available\n", format_size(size), m_id, format_size(m_size - m_used));
			else r2Error("Failed to allocate %s bytes from allocator %d, which has %s bytes available\n", format_size(size), m_id, format_size(m_size - m_used));

			exit(-1);
			return nullptr;
		}
		memory_block* block = find_available(size);

		assert(block != nullptr);

		block->used = m_id;
		m_used += size;
		return ptrFromBlock(block);
	}

	void* memory_allocator::reallocate(void* ptr, size_t size) {
		if (use_malloc) return realloc(ptr, size);
		memory_block* block = blockFromPtr(ptr);
		if (block->size == size) return ptr;
		if (block->used == m_id) {
			void* newPtr = reallocate_from_self(ptr, size);
			if (!newPtr) {
				if (m_id == 1) printf("Failed to reallocate %s bytes from allocator %d, which has %s bytes available\n", format_size(size), m_id, format_size(m_size - m_used));
				else r2Error("Failed to allocate %s bytes from allocator %d, which has %s bytes available\n", format_size(size), m_id, format_size(m_size - m_used));

				exit(-1);
				return nullptr;
			}

			return newPtr;
		}

		// this allocator didn't allocate ptr... nice
		if (block->used < m_id && m_last) {
			void* newPtr = m_last->reallocate_called_from_other_allocator(block, ptr, size, m_id);
			if (newPtr) return newPtr;
		}

		if (block->used > m_id && m_next) {
			void* newPtr = m_next->reallocate_called_from_other_allocator(block, ptr, size, m_id);
			if (newPtr) return newPtr;
		}

		r2Error("Memory leak detected. 0x%X was allocated by allocator %d, which was destroyed. Also, failed to reallocate data\n", ptr, block->used);
		exit(-1);
		return nullptr;
	}

	bool memory_allocator::deallocate(void* ptr) {
		if (use_malloc) {
			free(ptr);
			return true;
		}

		memory_block* block = blockFromPtr(ptr);
		if (block->used == m_id) {
			deallocate_from_self(ptr);
			return true;
		}

		// this allocator didn't allocate ptr... nice
		if (block->used < m_id && m_last && m_last->deallocate_called_from_other_allocator(block, ptr, m_id)) return true;
		if (block->used > m_id && m_next && m_next->deallocate_called_from_other_allocator(block, ptr, m_id)) return true;

		r2Error("Memory leak detected. 0x%X was allocated by allocator %d, which was destroyed.\n", ptr, block->used);
		return false;
	}

	void memory_allocator::deallocate_all() {
		m_baseBlock = (memory_block*)m_base;
		m_baseBlock->size = m_size - sizeof(memory_block);
		m_baseBlock->next = nullptr;
		m_baseBlock->used = false;
		m_used = sizeof(memory_block);
		m_blockCount = 1;
		m_mergeCounter = 50;
		memset(ptrFromBlock(m_baseBlock), 0xFC, m_baseBlock->size);
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

	void memory_allocator::debug(allocator_id level) {
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
	}

	allocator_id memory_allocator::id() const {
		return m_id;
	}

	void memory_allocator::deallocate_from_self(void* ptr) {
		if (use_malloc) {
			free(ptr);
			return;
		}
		memory_block* block = blockFromPtr(ptr);
		assert(block->used == m_id);

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

		// can't merge backwards here because of the lack of memory_block::last,
		// merge forward from the beginning instead every so often

		m_mergeCounter--;
		if (m_mergeCounter == 0) merge_adjacent_blocks();
	}

	bool memory_allocator::deallocate_called_from_other_allocator(memory_block* block, void* ptr, allocator_id otherAllocatorId) {
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

				block->size = size;

				memory_block* nextBlock = (memory_block*)(((u8*)ptr) + size);
				nextBlock->next = blockAfterNext;
				nextBlock->size = oldNextSize - sizeDiff;
				nextBlock->used = false;

				block->next = nextBlock;

				return ptr;
			} else {
				// nope. allocate a new one, copy this one to it, and deallocate this one
				void* newPtr = allocate(size);
				memcpy(newPtr, ptr, block->size);

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

				// can't merge backwards here because of the lack of memory_block::last,
				// merge forward from the beginning instead every so often

				m_mergeCounter--;
				if (m_mergeCounter == 0) merge_adjacent_blocks();

				return newPtr;
			}
		} else {
			size_t sizeDiff = block->size - size;
			
			// first, see if the next block can be expanded backwards
			if (block->next && !block->next->used) {
				size_t oldNextSize = block->next->size;
				memory_block* blockAfterNext = block->next->next;

				block->size = size;

				memory_block* nextBlock = (memory_block*)(((u8*)ptr) + size);
				nextBlock->next = blockAfterNext;
				nextBlock->size = oldNextSize + sizeDiff;
				nextBlock->used = false;

				block->next = nextBlock;

				return ptr;
			} else if (sizeDiff > sizeof(memory_block)) {
				// nope, but a new free block can be created in the gap left by shrinking this block

				memory_block* oldNextBlock = block->next;

				block->size = size;

				memory_block* nextBlock = (memory_block*)(((u8*)ptr) + size);
				nextBlock->next = oldNextBlock;
				nextBlock->size = sizeDiff - sizeof(memory_block);
				nextBlock->used = false;

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
					b->used = true;

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

		if (m_id == 1) printf("Failed to find memory for size %s\n", format_size(size));
		else r2Error("Failed to find memory for size %s\n", format_size(size));

		return nullptr;
	}


	memory_man* memory_man::get() {
		if (!memory_man::instance) {
			memory_man::instance = (memory_man*)malloc(sizeof(memory_man));
			new (memory_man::instance) memory_man();
		}
		return memory_man::instance;
	}

	memory_man::memory_man() {
		FILE* fp = fopen("mem.ini", "rb");
		if (!fp) {
			printf("No mem.ini found. Creating one, maximum memory usage by this application is now 32 MB. Take that.\n");
			fp = fopen("mem.ini", "w");
			if (fp) {
				m_memSize = MBtoB(32);
				fprintf(fp, "%llu", m_memSize);
				fclose(fp);
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

		m_base = malloc(m_memSize);
		m_baseAllocator.m_base = m_base;
		m_baseAllocator.m_size = m_memSize;
		m_baseAllocator.m_baseBlock = (memory_block*)m_base;
		m_baseAllocator.m_baseBlock->size = m_memSize - sizeof(memory_block);
		m_baseAllocator.m_baseBlock->next = nullptr;
		m_baseAllocator.m_baseBlock->used = false;
		m_baseAllocator.m_used = sizeof(memory_block);
		m_baseAllocator.m_blockCount++;
		m_allocatorStack = (allocator_stack*)m_baseAllocator.allocate(sizeof(allocator_stack));
		m_allocatorStack->last = nullptr;
		m_allocatorStack->next = nullptr;
		m_allocatorStack->allocator = &m_baseAllocator;
	}

	memory_man::~memory_man() {
		free(m_base);
	}

	memory_allocator* memory_man::get_allocator_by_id(allocator_id id) {
		memory_allocator* s = &m_baseAllocator;
		while(s) {
			if (s->m_id == id) return s;
			s = s->m_next;
		}
		return nullptr;
	}

	void memory_man::push_current(memory_allocator* allocator) {
		allocator_stack* s = (allocator_stack*)memory_man::instance->m_baseAllocator.allocate(sizeof(allocator_stack));
		s->last = nullptr;
		s->next = memory_man::instance->m_allocatorStack;
		s->allocator = allocator;
		memory_man::instance->m_allocatorStack->last = s;
		memory_man::instance->m_allocatorStack = s;
	}

	void memory_man::push_current(allocator_id allocatorId) {
		memory_allocator* allocator = memory_man::instance->get_allocator_by_id(allocatorId);
		if (!allocator) {
			r2Warn("Allocator with ID %d was destroyed or never existed. Using global allocator instead", allocatorId);
			memory_man::push_current(memory_man::global());
		} else memory_man::push_current(allocator);
	}

	memory_allocator* memory_man::pop_current() {
		allocator_stack* cur = memory_man::instance->m_allocatorStack;
		memory_allocator* allocator = cur->allocator;

		memory_man::instance->m_allocatorStack = cur->next;
		memory_man::instance->m_allocatorStack->last = nullptr;
		memory_man::instance->m_baseAllocator.deallocate_from_self(cur);
		return allocator;
	}

	memory_allocator* memory_man::get_deepest_allocator() {
		memory_allocator* s = &m_baseAllocator;
		while(s) {
			if (!s->m_next) return s;
			s = s->m_next;
		}

		return &m_baseAllocator;
	}

	memory_allocator* memory_man::current() { return memory_man::instance->m_allocatorStack->allocator; }

	memory_allocator* memory_man::global() {
		return &memory_man::instance->m_baseAllocator;
	}

	void* memory_man::allocate(size_t size) {
		return memory_man::get()->m_allocatorStack->allocator->allocate(size);
	}

	void* memory_man::reallocate(void* ptr, size_t size) {
		return memory_man::get()->m_allocatorStack->allocator->reallocate(ptr, size);
	}

	void memory_man::deallocate(void* ptr) {
		// will traverse the allocator list in both directions until the allocator that allocated this data is found
		// starts at current allocator, since that's probably the most common case
		memory_man::get()->m_allocatorStack->allocator->deallocate(ptr);
	}

	void memory_man::debug() {
		printf("\n\n\nmemory_man::debug()\n");
		memory_man::instance->m_baseAllocator.debug(0);
		memory_allocator* allocator = &memory_man::instance->m_baseAllocator;
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
	}

	void* r2alloc(size_t sz) {
		//void* ret = memory_man::get()->allocate(align(sz));
		//printf("r2alloc(%d:%d): 0x%X\n", sz, align(sz), (u64)ret);
		//return ret;
		return memory_man::allocate(align(sz));
	}

	void* r2realloc(void* ptr, size_t sz) {
		//void* ret = memory_man::get()->allocate(align(sz));
		//printf("r2alloc(%d:%d): 0x%X\n", sz, align(sz), (u64)ret);
		//return ret;
		return memory_man::reallocate(ptr, align(sz));
	}

	void r2free(void* ptr) {
		//printf("r2free: 0x%X\n", ptr);
		memory_man::deallocate(ptr);
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