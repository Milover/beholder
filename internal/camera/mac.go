// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

package camera

import "regexp"

var validMAC = regexp.MustCompile(`^([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2})$|^([0-9A-Fa-f]{12})$`)

// IsValidMAC verifies a MAC addresss.
func IsValidMAC(addr string) bool {
	return validMAC.MatchString(addr)
}
