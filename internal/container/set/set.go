// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

// Package set implements an unordered group of unique items.
//
// A Set[T] is a simple wrapper for a map[T]struct{}, so it can be used like
// a map and builtin functions like delete, clear and append work as expected.
// The methods of a Set are mostly wrappers around builtin functions and are
// provided for convenience.
//
// To iterate over a set, where s is a Set[T] and item is of type T:
//
//	for item := range s {
//		// do something with item
//	}
package set

// A Set is an unordered group of unique items of type T.
type Set[T comparable] map[T]struct{}

// New returns an initialized set.
func New[T comparable]() Set[T] {
	return make(map[T]struct{})
}

// Add adds an item t to the set s.
func (s *Set[T]) Add(t T) {
	(*s)[t] = struct{}{}
}

// Remove removes an t from s if t is an item in the set s.
func (s *Set[T]) Remove(t T) {
	delete((*s), t)
}

// Len returns the number of items in the set s.
func (s Set[T]) Len() int {
	return len(s)
}

// Clear deletes all entries in the set s, resulting in
// an empty set (s.Len() == 0).
func (s *Set[T]) Clear() {
	clear(*s)
}
