package mem

import (
	"runtime"
	"testing"
	"unsafe"

	"github.com/stretchr/testify/assert"
)

// TestRegion tests basic Region functionality.
// TODO: should make the test more thorough.
func TestRegion(t *testing.T) {
	assert := assert.New(t)

	runtime.LockOSThread()
	defer runtime.UnlockOSThread()

	var r Region

	nIter := 10000
	for i := 0; i < nIter; i++ {
		size := uint64(0)
		used := uint64(0)

		// test Alloc
		allocSize := uint64(0xffff)
		size = uint64(0x10000) // because will still be 0 when realloc occurs
		used += allocSize

		_ = r.Alloc(allocSize)
		assert.Equal(size, r.size, "bad size after Alloc")
		assert.Equal(used, r.used, "bad used space after Alloc")

		// test Copy
		b := []byte("test byte")
		size = 2 * size
		used += uint64(len(b))

		lCopy := r.Copy(b)
		assert.Equal(size, r.size, "bad size after Copy")
		assert.Equal(used, r.used, "bad used space after Copy")

		str := string(unsafe.Slice((*byte)(lCopy.Ptr()), len(b)))
		assert.Equal(string(b), str, "bad bytes after Copy")

		// test realloc
		size = 2 * r.size
		used += r.size

		_ = r.Alloc(r.size)
		assert.Equal(size, r.size, "bad size after realloc")
		assert.Equal(used, r.used, "bad used space after realloc")

		// force GC
		runtime.GC()

		// check if the bytes are still there
		str = string(unsafe.Slice((*byte)(lCopy.Ptr()), len(b)))
		assert.Equal(string(b), str, "bytes moved after realloc")

		// test Clear
		used = uint64(0)

		r.Clear()
		assert.Equal(size, r.size, "bad size after Clear")
		assert.Equal(used, r.used, "bad used space after Clear")

		// test Alloc
		used = allocSize

		_ = r.Alloc(allocSize)
		assert.Equal(size, r.size, "bad size after Clear-Alloc")
		assert.Equal(used, r.used, "bad used space after Clear-Alloc")

		// test Free
		size = uint64(0)
		used = uint64(0)

		r.Free()
		assert.Equal(unsafe.Pointer(nil), r.p, "bad pointer after Free")
		assert.Equal(size, r.size, "bad size after Free")
		assert.Equal(used, r.used, "bad used space after Free")
	}
}
