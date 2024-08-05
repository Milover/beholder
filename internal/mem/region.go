package mem

/*
#include <stdlib.h>
#include <string.h>
*/
import "C"
import "unsafe"

// Location is a location in a Region (block of memory).
type Location struct {
	offset uintptr // offset from the beginning of the region
	reg    *Region // pointer to region
}

// Ptr returns the current pointer to the memory location.
func (l Location) Ptr() unsafe.Pointer {
	return l.reg.at(l.offset)
}

// DefaultSize is the default Region size in bytes.
// TODO: should benchmark 256/512/1024.
const DefaultSize uint64 = 256

// A Region is a contiguous block of C-allocated memory.
//
// Requests for memory from a Region yield a [Location], which should
// be used to access the requested memory. The requested memory is
// uninitialized, as if returned by malloc.
//
// If there is enough free space within a Region to satisfy the request,
// existing Region space is reserved and no new memory is allocated.
// Otherwise, the Region will reallocate memory to satisfy the request.
// A Region's memory doubles on each reallocation.
//
// Note that reallocation, invalidates all pointers to a Region's memory.
// Hence it is unsafe to hold raw pointers to a Region's memory,
// use [Location.Ptr] instead.
//
// The zero value for a Region is a zero-sized block of memory ready to use.
// The Region's memory will be initialized to [DefaultSize] upon the first
// request for memory.
// To initialize a Region of a specific size use [NewRegion].
//
// WARNING: a Region holds a pointer to C-allocated memory,
// so when it is no longer needed, Free must be called
// to deallocate memory and clean up.
type Region struct {
	size uint64         // size of the memory block in bytes
	used uint64         // number of bytes of occupied memory in the block
	p    unsafe.Pointer // pointer to the beginning of the memory block
}

// NewRegion creates a new Region of the requested size by allocating
// a new block of memory.
func NewRegion(size uint64) *Region {
	return &Region{
		size: size,
		p:    C.malloc(C.size_t(size)),
	}
}

// NewRegion creates a new Region of default size by allocating
// a new block of memory.
func NewDefaultRegion() *Region {
	return &Region{
		size: DefaultSize,
		p:    C.malloc(C.size_t(DefaultSize)),
	}
}

// Alloc reserves memory in a region and returns it's location.
//
// If r does not have enough free space, r will reallocate such that
// the requested amount of memory can be reserved.
func (r *Region) Alloc(size uint64) Location {
	r.realloc(size)
	loc := Location{
		offset: uintptr(r.used),
		reg:    r,
	}
	r.used += size
	return loc
}

// at returns a pointer to a point within r, offset from the beginning of
// the block.
//
// If the returned pointer would point outside of r, the returned pointer
// will be nil.
func (r Region) at(offset uintptr) unsafe.Pointer {
	if uint64(offset) >= r.size {
		return nil
	}
	return unsafe.Add(r.p, offset)
}

// Clear frees r's used space.
// The size of r remains unchanged and the underlying memory is not deallocated.
//
// Old locations within r should not be used after Clear.
func (r *Region) Clear() {
	r.used = 0
}

// Copy copies bytes into r and returns the location at which
// the bytes were written.
//
// If r does not have enough free space to hold the bytes,
// r will reallocate memory such that the bytes can be copied.
func (r *Region) Copy(b []byte) Location {
	loc := r.Alloc(uint64(len(b)))
	C.memcpy(loc.Ptr(), unsafe.Pointer(&b[0]), C.size_t(len(b)))
	return loc
}

// Free deallocates the memory region.
//
// Old locations within r should not be used after Free.
func (r *Region) Free() {
	if r.p != nil {
		C.free(r.p)
	}
	*r = Region{}
}

// realloc reallocates a regions memory if necessary.
//
// If r has enough free space to hold an additional req bytes, the function
// does nothing.
// If r does not have enough free space, r's size is doubled as many times
// as necessary such that an additional req bytes can be stored. Once a
// sufficient size is found, r is reallocated.
//
// realloc will panic if reallocation fails.
func (r *Region) realloc(req uint64) {
	resize := r.size
	if resize == 0 {
		resize = DefaultSize
	}
	for req+r.used >= resize {
		resize = 2 * resize
	}
	if resize != r.size {
		r.size = resize
		ptr := r.p // in case we fail, this will still be valid
		if r.p = C.realloc(r.p, C.size_t(r.size)); r.p == nil {
			C.free(ptr)
			panic("C realloc failed")
		}
	}
}
