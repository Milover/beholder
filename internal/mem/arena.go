package mem

/*
#include <stdlib.h>
#include <string.h>
*/
import "C"
import "unsafe"

// An Arena manages C-allocated memory.
//
// Arena assumes ownership of all of it's pointers, hence when Free is called
// all memory accessable through the Arena is freed.
//
// The zero value for an Arena is a ready-to-use zero-sized array of pointers
// to C-allocated memory.
// Once initialized it should not be copied.
//
// WARNING: Arena owns C-allocated memory, so when it is no longer needed,
// Free must be called to deallocate memory and clean up.
type Arena []unsafe.Pointer

// Malloc allocates memory in ar and returns a pointer to the memory.
func (ar *Arena) Malloc(size uint64) unsafe.Pointer {
	ptr := C.malloc(C.size_t(size))
	*ar = append(*ar, ptr)
	return ptr
}

// Copy copies bytes into ar and returns a pointer to the location
// at which the bytes were written.
func (ar *Arena) Copy(b []byte) unsafe.Pointer {
	ptr := ar.Malloc(uint64(len(b)))
	C.memcpy(ptr, unsafe.Pointer(&b[0]), C.size_t(len(b)))
	return ptr
}

// CopyStr copies a string into ar and returns a pointer to the location
// at which the string was written.
//
// The copied string is always null-terminated.
func (ar *Arena) CopyStr(s string) unsafe.Pointer {
	size := uint64(len(s) + 1)
	ptr := ar.Malloc(size)
	C.memcpy(ptr, unsafe.Pointer(&([]byte(s))[0]), C.size_t(size))
	unsafe.Slice((*byte)(ptr), size)[size-1] = '\x00'
	return ptr
}

// CopyCStr copies a C-string (*C.char) into ar and returns
// the location at which the string was written.
//
// It is assumed that the cstr is null-terminated.
func (ar *Arena) CopyCStr(cstr unsafe.Pointer) unsafe.Pointer {
	size := uint64(C.strlen((*C.char)(cstr))) + 1
	ptr := ar.Malloc(size)
	C.memcpy(ptr, cstr, C.size_t(size))
	return ptr
}

// Free deallocates all memory owned by ar and sets it's size to zero.
func (ar *Arena) Free() {
	for _, p := range *ar {
		C.free(p)
	}
	*ar = (*ar)[:0]
}

// Store appends p to ar, which then assumes ownership of p.
func (ar *Arena) Store(p unsafe.Pointer) unsafe.Pointer {
	*ar = append(*ar, p)
	return p
}
