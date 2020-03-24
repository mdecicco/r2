#include <r2/managers/memman.h>
using namespace r2;

namespace robin_hood {
	void* rh_malloc(size_t size) {
		return memory_man::global()->allocate(size);
	}
	void* rh_calloc(size_t count, size_t size) {
		void* mem = memory_man::global()->allocate(count * size);
		memset(mem, 0, count * size);
		return mem;
	}
	void rh_free(void* ptr) {
		memory_man::global()->deallocate(ptr);
	}
};