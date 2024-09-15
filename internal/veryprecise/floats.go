// Package veryprecise defines utilities for very precise™ floating-point
// comparison operations.
package veryprecise

import (
	"math"
)

// The minimum normalized positive values of floating-point types.
const (
	MinFloat32 = 0x1p-126  // equivalent to C's FLT_MIN
	MinFloat64 = 0x1p-1022 // equivalent to C's DBL_MIN
)

// The lowest negative integers n, such that 2**n, is a valid normalized value
// of a given floating-point type.
const (
	MinExponentFloat32 = -126  // equivalent to C's FLT_MIN_EXP + 1
	MinExponentFloat64 = -1022 // equivalent to C's DBL_MIN_EXP + 1
)

// Machine epsilons for floating-point types, that is, difference between 1.0
// and the next value representable by a given floating-point type
var (
	EpsilonFloat32 = math.Nextafter32(1.0, 2.0) - 1.0 // equivalent to C's FLT_EPSILON
	EpsilonFloat64 = math.Nextafter(1.0, 2.0) - 1.0   // equivalent to C's DBL_EPSILON
)

// EqualULP reports whether two float64 values, x and y, are equal to within
// ulp units in the last place (ULPs).
func EqualULP(x, y float64, ulp int) bool {
	return equalULP(
		x, y, ulp,
		MinFloat64,
		EpsilonFloat64,
		MinExponentFloat64,
	)
}

// EqualULP32 reports whether two float32 values, x and y, are equal to within
// ulp units in the last place (ULPs).
func EqualULP32(x, y float32, ulp int) bool {
	return equalULP(
		float64(x), float64(y), ulp,
		float64(MinFloat32),
		float64(EpsilonFloat32),
		MinExponentFloat32,
	)
}

// equalULP is a helper function which implements the equality check of two
// floating-point values, up to ulp units in the last place (ULPs).
//
// This implementation was taken directly from an [example on the C++ reference]
// site.
//
// [example on the C++ reference]: https://en.cppreference.com/w/cpp/types/numeric_limits/epsilon
func equalULP(x, y float64, ulp int, smallest, eps float64, minExp int) bool {
	// Since eps is the gap size (ULP, unit in the last place)
	// of floating-point numbers in interval [1, 2), we can scale it to
	// the gap size in the interval [2**e, 2**(e+1)), where 'e' is the exponent
	// of x and y.

	// If x and y have different gap sizes (which means they have
	// different exponents), we take the smaller one.
	// Taking the bigger one is also reasonable, I guess.
	m := min(math.Abs(x), math.Abs(y))

	// Subnormal numbers have fixed exponent, which is minExp.
	var exp int
	if m < smallest {
		exp = minExp
	} else {
		exp = math.Ilogb(m)
	}

	// We consider x and y equal if the difference between them is
	// within ulp ULPs.
	return math.Abs(x-y) <= float64(ulp)*math.Ldexp(eps, exp)
}
