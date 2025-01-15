#ifndef _BEHOLDER_MEM_H
#define _BEHOLDER_MEM_H

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
typedef void (*Deleter)(void*);
typedef void* Ptr;
#else
typedef void (*Deleter)(void*);
typedef void* Ptr;
#endif

// Mem_Delete is a function which deletes ptr by calling the deleter del
// with ptr as the argument.
void Mem_Delete(Ptr ptr, Deleter del);

// Mem_DeleteCharPtr is a convenience function which acts as a deleter
// for a char* allocated with new[].
void Mem_DeleteCharPtr(Ptr ptr);

// Mem_DeleteCharPtrArr is a convenience function which acts as a deleter
// for a char** allocated with new[].
void Mem_DeleteCharPtrArr(Ptr ptr);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif	// _BEHOLDER_MEM_H
