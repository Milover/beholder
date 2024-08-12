// Package internal contains definitions used for testing mem.
package internal

/*
#cgo CXXFLAGS: -std=c++17
#include <stdlib.h>
#include "internal.h"
*/
import "C"
import (
	"unsafe"
)

// NewCounter returns a C-allocated pointer to a Counter.
func NewCounter() unsafe.Pointer {
	return unsafe.Pointer(C.Count_New())
}

// NewCounterArray returns a pointer to a C-allocated array of count Counters.
func NewCounterArray(count uint64) unsafe.Pointer {
	return unsafe.Pointer(C.Count_NewArray(C.size_t(count)))
}

// NewCounterPtrArray returns a pointer to a C-allocated array of count
// pointers to Counters.
func NewCounterPtrArray(count uint64) unsafe.Pointer {
	return unsafe.Pointer(C.Count_NewPtrArray(C.size_t(count)))
}

func Alive() uint64 {
	return uint64(C.Count_Alive())
}

func Constructed() uint64 {
	return uint64(C.Count_Constructed())
}

func Destructed() uint64 {
	return uint64(C.Count_Destructed())
}

// CounterArrDeleter is a convenience function which sets p's deleter
// to C.Count_DeleteArr.
//
// It is necessary because C functions can't be called from tests.
func CounterArrDeleter() unsafe.Pointer {
	return unsafe.Pointer(C.Count_DeleteArr)
}

// CounterDeleter is a convenience function which sets p's deleter
// to C.Count_Delete.
//
// It is necessary because C functions can't be called from tests.
func CounterDeleter() unsafe.Pointer {
	return unsafe.Pointer(C.Count_Delete)
}

// CounterPtrArrDeleter is a convenience function which sets p's deleter
// to C.Count_DeletePtrArr.
//
// It is necessary because C functions can't be called from tests.
func CounterPtrArrDeleter() unsafe.Pointer {
	return unsafe.Pointer(C.Count_DeletePtrArr)
}
