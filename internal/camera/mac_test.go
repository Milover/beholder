// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

//go:build linux

package camera

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

// Test MAC address validation.
type macAddrTest struct {
	Name     string
	Input    string
	Expected bool
}

var macAddrTests = []macAddrTest{
	{
		Name:     "good-colons",
		Input:    "01:23:45:67:89:AB",
		Expected: true,
	},
	{
		Name:     "good-colons-lowercase",
		Input:    "01:f3:ab:67:89:ab",
		Expected: true,
	},
	{
		Name:     "good-hyphens",
		Input:    "01-23-45-67-89-AB",
		Expected: true,
	},
	{
		Name:     "good-no-separator",
		Input:    "0123456789AB",
		Expected: true,
	},
	{
		Name:     "bad-separator",
		Input:    "0123.4567.89AB",
		Expected: false,
	},
	{
		Name:     "bad-byte-value-colons",
		Input:    "01:23:45:67:89:ZZ",
		Expected: false,
	},
	{
		Name:     "bad-byte-value-no-separators",
		Input:    "0123456789XYZ",
		Expected: false,
	},
}

func TestIsValidMAC(t *testing.T) {
	for _, tt := range macAddrTests {
		t.Run(tt.Name, func(t *testing.T) {
			assert := assert.New(t)
			assert.Equal(tt.Expected, IsValidMAC(tt.Input))
		})
	}
}
