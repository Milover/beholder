// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#ifndef _BEHOLDER_MEM_INTERNAL_H
#define _BEHOLDER_MEM_INTERNAL_H

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
struct Counter;	 // forward declaration

typedef Counter* Count;
#else
typedef void* Count;
#endif

size_t Count_Alive();
size_t Count_Constructed();
void Count_DeleteArr(void* c);
void Count_Delete(void* c);
void Count_DeletePtrArr(void* c);
size_t Count_Destructed();
Count Count_New();
Count Count_NewArray(size_t count);
Count* Count_NewPtrArray(size_t count);
void Count_Reset();	 // XXX: spooky

#ifdef __cplusplus
}  // extern "C"
#endif

#endif	// _BEHOLDER_MEM_INTERNAL_H
