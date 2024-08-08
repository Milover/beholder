#include <cstdlib>

#include "arena.h"

void Mem_Delete(Ptr* ptr, Deleter del) {
	if (ptr && del) {
		del(ptr);
	}
}

void Mem_DeleteCharPtr(Ptr ptr) {
	if (ptr) {
		char** p {static_cast<char**>(ptr)};
		if (*p) {
			delete[] *p;
			*p = nullptr;
		}
	}
}
