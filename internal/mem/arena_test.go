// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

package mem

import (
	"runtime"
	"testing"

	"github.com/Milover/beholder/internal/mem/internal"
	"github.com/stretchr/testify/assert"
)

// ctrAssert is a helper for asserting Counter counts.
type ctrAssert struct {
	alive int
	cnstr int
	destr int
	ass   *assert.Assertions
	tst   *testing.T
}

// Assert asserts the current counts stored in c.
func (c ctrAssert) Assert() {
	c.tst.Helper()
	c.ass.Equal(uint64(c.alive), internal.Alive(), "alive mismatch")
	c.ass.Equal(uint64(c.cnstr), internal.Constructed(), "constructed mismatch")
	c.ass.Equal(uint64(c.destr), internal.Destructed(), "destructed mismatch")
}

// AssertWithGC asserts the current counts stored in c, then forces garbage
// collection and then asserts the counts again.
func (c ctrAssert) AssertWithGC() {
	c.tst.Log("asserting before GC")
	c.Assert()

	runtime.GC()

	c.tst.Log("asserting after GC")
	c.Assert()
}

// TestStorage validates operation of [Arena.Store] and [Arena.StoreArray].
//
// TODO: test C-string stuff.
func TestStorage(t *testing.T) {
	t.Parallel()

	// make sure we reset all counters before and after the test
	internal.Reset()
	defer internal.Reset()

	ca := ctrAssert{
		ass: assert.New(t),
		tst: t,
	}
	ar := &Arena{}
	defer ar.Free()

	// sanity check
	ca.Assert()

	// test with a simple pointer
	ca.tst.Log("Array.Store(ptr to Counter)")
	ar.Store(internal.NewCounter(), internal.CounterDeleter())

	ca.alive += 1
	ca.cnstr += 1
	ca.AssertWithGC()

	ca.tst.Log("Array.Free()")
	ar.Free()

	ca.alive -= 1
	ca.destr += 1
	ca.AssertWithGC()

	count := 10 // the number of elements in the array

	// test with a pointer to an array (of values)
	ca.tst.Logf("Array.Store(ptr to Counter[%d])", count)
	ar.Store(internal.NewCounterArray(uint64(count)), internal.CounterArrDeleter())

	ca.alive += count
	ca.cnstr += count
	ca.AssertWithGC()

	ca.tst.Log("Array.Free()")
	ar.Free()

	ca.alive -= count
	ca.destr += count
	ca.AssertWithGC()

	// test with a pointer to an array of pointers
	ca.tst.Logf("Array.StoreArray(ptr to Counter-ptr[%d])", count)
	ar.StoreArray(
		internal.NewCounterPtrArray(uint64(count)),
		internal.CounterPtrArrDeleter(),
		internal.CounterDeleter(),
		uint64(count),
	)

	ca.alive += count
	ca.cnstr += count
	ca.AssertWithGC()

	ca.tst.Log("Array.Free()")
	ar.Free()

	ca.alive -= count
	ca.destr += count
	ca.AssertWithGC()
}
