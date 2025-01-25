// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#include "arena.h"

#include <cstdlib>

void Mem_Delete(Ptr ptr, Deleter del) {
	if (ptr && del) {
		del(ptr);
	}
}

void Mem_DeleteCharPtr(Ptr ptr) {
	if (ptr) {
		char** p{static_cast<char**>(ptr)};
		if (*p) {
			delete[] *p;
			*p = nullptr;
		}
	}
}

void Mem_DeleteCharPtrArr(Ptr ptr) {
	if (ptr) {
		// a pointer to an array-pointer of char*
		// FIXME: wtf bro
		char*** p{static_cast<char***>(ptr)};
		if (*p) {
			delete[] *p;
			*p = nullptr;
		}
	}
}
