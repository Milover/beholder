#include "internal.h"

#include <cstdlib>

// Counter is a class which keeps track of it's allocations and deallocations.
struct Counter {
	// alive is the number of currently alive Counters (non-destructed).
	inline static size_t alive{0};
	// constructed is the total number of Counters ever constructed.
	inline static size_t constructed{0};
	// destructed is the total number of Counters ever destructed.
	inline static size_t destructed{0};

	// Default constructor
	Counter() {
		++alive;
		++constructed;
	}

	// Destructor
	~Counter() {
		--alive;
		++destructed;
	}
};

size_t Count_Alive() { return Counter::alive; }

size_t Count_Constructed() { return Counter::constructed; }

void Count_Delete(void* c) {
	if (c) {
		// a pointer to a Counter pointer
		Counter** ptr{static_cast<Counter**>(c)};
		if (*ptr) {
			delete *ptr;
			*ptr = nullptr;
		}
	}
}

void Count_DeleteArr(void* c) {
	if (c) {
		// a pointer to an array-pointer of Counters
		Counter** ptr{static_cast<Counter**>(c)};
		if (*ptr) {
			delete[] *ptr;
			*ptr = nullptr;
		}
	}
}

void Count_DeletePtrArr(void* c) {
	if (c) {
		// a pointer to an array-pointer of Counter pointers
		// FIXME: wtf bro
		Counter*** ptr{static_cast<Counter***>(c)};
		if (*ptr) {
			delete[] *ptr;
			*ptr = nullptr;
		}
	}
}

size_t Count_Destructed() { return Counter::destructed; }

Count Count_New() { return new Counter{}; }

Count Count_NewArray(size_t count) { return new Counter[count]; }

Count* Count_NewPtrArray(size_t count) {
	Counter** arr{new Counter*[count]};
	for (auto i{0ul}; i < count; ++i) {
		arr[i] = new Counter{};
	}
	return arr;
}
