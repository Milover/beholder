// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

package veryprecise

import (
	"fmt"
	"math"
	"testing"

	"github.com/stretchr/testify/assert"
	"golang.org/x/exp/constraints"
)

type equalULPTest[T constraints.Float] struct {
	X, Y T    // the values to compare
	ULP  int  // units in the last place up to which the comparison is done
	Exp  bool // the expected comparison result
}

// Test the float64 version of EqualULP for various cases.
var equalULPTestsFloat64 = []equalULPTest[float64]{
	// same, non-zero numbers
	equalULPTest[float64]{X: 0.1, Y: 0.1, ULP: 0, Exp: true},
	equalULPTest[float64]{X: 0.1, Y: 0.1, ULP: 1, Exp: true},
	equalULPTest[float64]{X: 0.1, Y: 0.1, ULP: 2, Exp: true},
	// different, non-zero numbers at ulp = 1
	equalULPTest[float64]{X: 1.0, Y: 0x1.0000000000001p+00, ULP: 0, Exp: false},
	equalULPTest[float64]{X: 1.0, Y: 0x1.0000000000001p+00, ULP: 1, Exp: true},
	equalULPTest[float64]{X: 1.0, Y: 0x1.0000000000001p+00, ULP: 2, Exp: true},
	// different, non-zero numbers at ulp = 2
	equalULPTest[float64]{X: 1.0, Y: 0x1.0000000000002p+00, ULP: 0, Exp: false},
	equalULPTest[float64]{X: 1.0, Y: 0x1.0000000000002p+00, ULP: 1, Exp: false},
	equalULPTest[float64]{X: 1.0, Y: 0x1.0000000000002p+00, ULP: 2, Exp: true},
	// different, non-zero numbers in decimal form at specific ulps
	equalULPTest[float64]{X: 1.0, Y: 1.00000000000000000, ULP: 0, Exp: true},
	equalULPTest[float64]{X: 1.0, Y: 1.00000000000000001, ULP: 1, Exp: true},
	equalULPTest[float64]{X: 1.0, Y: 1.00000000000000010, ULP: 2, Exp: true},
	// different, non-zero subnormal numbers
	equalULPTest[float64]{X: 0.1 * MinFloat64, Y: math.Nextafter(0.1*MinFloat64, 1.0), ULP: 0, Exp: false},
	equalULPTest[float64]{X: 0.1 * MinFloat64, Y: math.Nextafter(0.1*MinFloat64, 1.0), ULP: 1, Exp: true},
	equalULPTest[float64]{X: 0.1 * MinFloat64, Y: math.Nextafter(0.1*MinFloat64, 1.0), ULP: 2, Exp: true},
	// different, non-zero subnormal numbers with subnormal difference
	equalULPTest[float64]{X: math.SmallestNonzeroFloat64, Y: math.Nextafter(math.SmallestNonzeroFloat64, 1.0), ULP: 0, Exp: false},
	equalULPTest[float64]{X: math.SmallestNonzeroFloat64, Y: math.Nextafter(math.SmallestNonzeroFloat64, 1.0), ULP: 1, Exp: true},
	equalULPTest[float64]{X: math.SmallestNonzeroFloat64, Y: math.Nextafter(math.SmallestNonzeroFloat64, 1.0), ULP: 2, Exp: true},
	// zero and non-zero
	equalULPTest[float64]{X: 0.0, Y: MinFloat64, ULP: 0, Exp: false},
	equalULPTest[float64]{X: 0.0, Y: MinFloat64, ULP: 1, Exp: false},
	equalULPTest[float64]{X: 0.0, Y: MinFloat64, ULP: 2, Exp: false},
	// zero and subnormal non-zero
	equalULPTest[float64]{X: 0.0, Y: math.Nextafter(0.0, 1.0), ULP: 0, Exp: false},
	equalULPTest[float64]{X: 0.0, Y: math.Nextafter(0.0, 1.0), ULP: 1, Exp: true},
	equalULPTest[float64]{X: 0.0, Y: math.Nextafter(0.0, 1.0), ULP: 2, Exp: true},
}

func TestEqualULPFloat64(t *testing.T) {
	for i, tt := range equalULPTestsFloat64 {
		t.Run(fmt.Sprintf("test-%d", i), func(t *testing.T) {
			assert := assert.New(t)

			assert.Equalf(tt.Exp, EqualULP(tt.X, tt.Y, tt.ULP),
				"x: %.13x (%.17e)\ny: %.13x (%.17e)", tt.X, tt.X, tt.Y, tt.Y)
		})
	}
}

// Test the float32 version of EqualULP32 for various cases.
var equalULPTestsFloat32 = []equalULPTest[float32]{
	// same, non-zero numbers
	equalULPTest[float32]{X: 0.1, Y: 0.1, ULP: 0, Exp: true},
	equalULPTest[float32]{X: 0.1, Y: 0.1, ULP: 1, Exp: true},
	equalULPTest[float32]{X: 0.1, Y: 0.1, ULP: 2, Exp: true},
	// different, non-zero numbers at ulp = 1
	equalULPTest[float32]{X: 1.0, Y: 0x1.000002p+00, ULP: 0, Exp: false},
	equalULPTest[float32]{X: 1.0, Y: 0x1.000002p+00, ULP: 1, Exp: true},
	equalULPTest[float32]{X: 1.0, Y: 0x1.000002p+00, ULP: 2, Exp: true},
	// different, non-zero numbers at ulp = 2
	equalULPTest[float32]{X: 1.0, Y: 0x1.000004p+00, ULP: 0, Exp: false},
	equalULPTest[float32]{X: 1.0, Y: 0x1.000004p+00, ULP: 1, Exp: false},
	equalULPTest[float32]{X: 1.0, Y: 0x1.000004p+00, ULP: 2, Exp: true},
	// different, non-zero numbers in decimal form at specific ulps
	equalULPTest[float32]{X: 1.0, Y: 1.000000000, ULP: 0, Exp: true},
	equalULPTest[float32]{X: 1.0, Y: 1.000000001, ULP: 1, Exp: true},
	equalULPTest[float32]{X: 1.0, Y: 1.000000010, ULP: 2, Exp: true},
	// different, non-zero subnormal numbers
	equalULPTest[float32]{X: 0.1 * MinFloat32, Y: math.Nextafter32(0.1*MinFloat32, 1.0), ULP: 0, Exp: false},
	equalULPTest[float32]{X: 0.1 * MinFloat32, Y: math.Nextafter32(0.1*MinFloat32, 1.0), ULP: 1, Exp: true},
	equalULPTest[float32]{X: 0.1 * MinFloat32, Y: math.Nextafter32(0.1*MinFloat32, 1.0), ULP: 2, Exp: true},
	// different, non-zero subnormal numbers with subnormal difference
	equalULPTest[float32]{X: math.SmallestNonzeroFloat32, Y: math.Nextafter32(math.SmallestNonzeroFloat32, 1.0), ULP: 0, Exp: false},
	equalULPTest[float32]{X: math.SmallestNonzeroFloat32, Y: math.Nextafter32(math.SmallestNonzeroFloat32, 1.0), ULP: 1, Exp: true},
	equalULPTest[float32]{X: math.SmallestNonzeroFloat32, Y: math.Nextafter32(math.SmallestNonzeroFloat32, 1.0), ULP: 2, Exp: true},
	// zero and non-zero
	equalULPTest[float32]{X: 0.0, Y: MinFloat32, ULP: 0, Exp: false},
	equalULPTest[float32]{X: 0.0, Y: MinFloat32, ULP: 1, Exp: false},
	equalULPTest[float32]{X: 0.0, Y: MinFloat32, ULP: 2, Exp: false},
	// zero and subnormal non-zero
	equalULPTest[float32]{X: 0.0, Y: math.Nextafter32(0.0, 1.0), ULP: 0, Exp: false},
	equalULPTest[float32]{X: 0.0, Y: math.Nextafter32(0.0, 1.0), ULP: 1, Exp: true},
	equalULPTest[float32]{X: 0.0, Y: math.Nextafter32(0.0, 1.0), ULP: 2, Exp: true},
}

func TestEqualULPFloat32(t *testing.T) {
	for i, tt := range equalULPTestsFloat32 {
		t.Run(fmt.Sprintf("test-%d", i), func(t *testing.T) {
			assert := assert.New(t)

			assert.Equalf(tt.Exp, EqualULP32(tt.X, tt.Y, tt.ULP),
				"x: %.6x (%.9e)\ny: %.6x (%.9e)", tt.X, tt.X, tt.Y, tt.Y)
		})
	}
}
