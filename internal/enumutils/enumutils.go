// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

package enumutils

import (
	"encoding/json"
	"fmt"
)

// Invert returns a map which is the inverse of m.
// The keys of the inverse are the values of m, and the values of the inverse
// are the keys of m.
//
// This is most useful when a predefined map needs to be searched by value.
func Invert[M ~map[K]V, K comparable, V comparable](m M) map[V]K {
	inv := make(map[V]K, len(m))
	for k, v := range m {
		inv[v] = k
	}
	return inv
}

// UnmarshalJSON is a helper function which implements JSON unmarshalling
// for named enumerations.
func UnmarshalJSON[T comparable](data []byte, t *T, m map[string]T) error {
	var s string
	if err := json.Unmarshal(data, &s); err != nil {
		return err
	}
	typ, ok := m[s]
	if !ok {
		return fmt.Errorf("bad %T key: %q", *t, s)
	}
	*t = typ
	return nil
}

// MarshalJSON is a helper function which implements JSON marshalling
// for named enumerations.
func MarshalJSON[T comparable](t T, m map[T]string) ([]byte, error) {
	key, ok := m[t]
	if !ok {
		return nil, fmt.Errorf("bad %T value: %v", t, t)
	}
	return json.Marshal(key)
}
