package maputils

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
