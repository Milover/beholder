// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

// Package mem provides utilities for managing C-allocated memory.
package mem

/*
#include <stdlib.h>
#include <string.h>
#include "arena.h"
*/
import "C"
import "unsafe"

// Sizes, in bytes, of commonly used C-types.
const (
	SizeofCharPtr = uint64(unsafe.Sizeof((*C.char)(nil)))
)

// A Pointer enables correct deletion of a C-allocated value.
type Pointer struct {
	// Del is a function pointer of the type: void (*func)(void*), which
	// correctly frees resources held by Ptr when called.
	//
	// If Del is nil, it is assumed that Ptr was allocated by
	// C.malloc/C.calloc/C.realloc and no special cleanup is necessary, i.e.
	// Ptr is freed by calling C.free.
	Del unsafe.Pointer
	// Ptr is the pointer to the C-allocated value.
	Ptr unsafe.Pointer
}

// Delete frees resources held by p by calling it's deleter.
func (p *Pointer) Delete() {
	if p.Del != nil {
		C.Mem_Delete(C.Ptr(unsafe.Pointer(&p.Ptr)), C.Deleter(p.Del))
	} else {
		C.free(p.Ptr)
		p.Ptr = nil
	}
}

// An Arena manages C-allocated memory.
//
// Arena assumes ownership of all of it's pointers, hence when Free is called
// all memory accessible through the Arena is freed.
//
// The zero value for an Arena is a ready-to-use zero-sized array of pointers
// to C-allocated memory.
// Once initialized it should not be copied.
//
// WARNING: Arena owns C-allocated memory, so when it is no longer needed,
// Free must be called to deallocate memory and clean up.
type Arena []Pointer

// Malloc allocates memory in ar and returns a pointer to the memory.
func (ar *Arena) Malloc(size uint64) unsafe.Pointer {
	ptr := C.malloc(C.size_t(size))
	*ar = append(*ar, Pointer{Ptr: ptr})
	return ptr
}

// Copy copies bytes into ar and returns a pointer to the location
// at which the bytes were written.
func (ar *Arena) Copy(b []byte) unsafe.Pointer {
	size := uint64(len(b))
	ptr := ar.Malloc(size)
	if size > uint64(1) {
		C.memcpy(ptr, unsafe.Pointer(&b[0]), C.size_t(len(b)))
	}
	return ptr
}

// CopyStr copies a string into ar and returns a (*C.char) pointer to the
// location at which the string was written.
//
// The copied string is always null-terminated.
func (ar *Arena) CopyStr(s string) unsafe.Pointer {
	size := uint64(len(s) + 1)
	ptr := ar.Malloc(size)
	if size > uint64(1) {
		C.memcpy(ptr, unsafe.Pointer(&([]byte(s))[0]), C.size_t(size))
	}
	unsafe.Slice((*byte)(ptr), size)[size-1] = '\x00'
	return ptr
}

// CopyStrArray copies all elements of ss into ar and
// returns a (**C.char) pointer to the location at which the first element
// (*C.char) was written.
//
// The copied strings are always null-terminated.
func (ar *Arena) CopyStrArray(ss []string) unsafe.Pointer {
	nStrings := uint64(len(ss))
	strings := (**C.char)(ar.Malloc(nStrings * SizeofCharPtr))
	stringsSlice := unsafe.Slice(strings, nStrings)
	for i, s := range ss {
		stringsSlice[i] = (*C.char)(ar.CopyStr(s))
	}
	return unsafe.Pointer(strings)
}

// Free deallocates all memory owned by ar and sets it's size to zero.
func (ar *Arena) Free() {
	for _, p := range *ar {
		p.Delete()
	}
	*ar = (*ar)[:0]
}

// Store appends p and it's deleter del to ar, which then assumes
// ownership of p. p is freed by calling del with p as the argument.
//
// p is returned for convenience.
func (ar *Arena) Store(p, del unsafe.Pointer) unsafe.Pointer {
	*ar = append(*ar, Pointer{Ptr: p, Del: del})
	return p
}

// StoreArray appends a pointer to an array of count pointers ptrs, and
// all elements (pointers) within ptrs, to ar, and assumes ownership
// of both ptrs and all elements of ptrs.
// ptrs is freed by calling delP with ptrs as the argument, while
// the elements of ptrs are freed by sequentially calling delEl with each
// element of ptrs as the argument.
//
// ptrs is returned for convenience.
//
// WARNING: the elements of ptrs must be pointers to C-allocated values.
func (ar *Arena) StoreArray(ptrs, delP, delEl unsafe.Pointer, count uint64) unsafe.Pointer {
	s := unsafe.Slice((*unsafe.Pointer)(ptrs), count)
	for _, ptr := range s {
		*ar = append(*ar, Pointer{Ptr: ptr, Del: delEl})
	}
	*ar = append(*ar, Pointer{Ptr: ptrs, Del: delP})
	return ptrs
}

// StoreCStr stores the C-string (*C.char) cstr into ar, which then assumes
// owhership of cstr. cstr is freed as if though it were allocated
// by calling new[].
//
// cstr is returned for convenience.
func (ar *Arena) StoreCStr(cstr unsafe.Pointer) unsafe.Pointer {
	return ar.Store(cstr, C.Mem_DeleteCharPtr)
}

// StoreCStrConv stores the C-string (*C.char) cstr into ar, which then assumes
// owhership of cstr. cstr is freed as if though it were allocated
// by calling new[].
//
// A Go string is returned for convenience.
func (ar *Arena) StoreCStrConv(cstr unsafe.Pointer) string {
	return C.GoString((*C.char)(ar.StoreCStr(cstr)))
}

// StoreCStrArray stores the C-string array (**C.char) cstr into ar, which
// then assumes owhership of cstr. cstr is freed as if though it were allocated
// by calling new[].
//
// cstr is returned for convenience.
func (ar *Arena) StoreCStrArray(cstr unsafe.Pointer, count uint64) unsafe.Pointer {
	return ar.StoreArray(
		cstr,
		C.Mem_DeleteCharPtrArr,
		C.Mem_DeleteCharPtr,
		count,
	)
}

// StoreCStrArrayConv stores the C-string array (**C.char) cstr of size count
// into ar, which then assumes owhership of cstr. cstr and all it's elements
// are freed as if though they were allocated by calling new[].
//
// A slice of Go strings is returned for convenience.
func (ar *Arena) StoreCStrArrayConv(cstr unsafe.Pointer, count uint64) []string {
	strs := make([]string, 0, count)
	chs := unsafe.Slice((**C.char)(cstr), count)
	for _, ptr := range chs {
		*ar = append(*ar, Pointer{
			Ptr: unsafe.Pointer(ptr),
			Del: C.Mem_DeleteCharPtr,
		})
		strs = append(strs, C.GoString(ptr))
	}
	*ar = append(*ar, Pointer{
		Ptr: cstr,
		Del: C.Mem_DeleteCharPtrArr,
	})
	return strs
}
