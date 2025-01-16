// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

package models

import (
	"fmt"
	"math"
)

// A Rectangle has four vertices.
type Rectangle struct {
	Left, Top, Right, Bottom int64 // the vertices
}

// Area computes the area of r.
func (r Rectangle) Area() int64 {
	return (r.Right - r.Left) * (r.Bottom - r.Top)
}

// Height returns the width of r.
func (r Rectangle) Height() int64 {
	return r.Bottom - r.Top
}

// In reports whether every point in r is in s.
func (r Rectangle) In(s Rectangle) bool {
	return r.Left > s.Left &&
		r.Top > s.Top &&
		r.Right < s.Right &&
		r.Bottom < s.Bottom
}

// Rotate90 rotates r by 90°. Note that due to how [Rectangle] is implemented,
// rotating by +/-90° and +/-270° yields the same r in all cases.
func (r *Rectangle) Rotate90() {
	w := r.Right - r.Left
	h := r.Bottom - r.Top
	cx := 0.5 * float64(r.Right+r.Left)
	cy := 0.5 * float64(r.Bottom+r.Top)

	r.Left = int64(math.Floor(cx - 0.5*float64(h)))
	r.Top = int64(math.Floor(cy - 0.5*float64(w)))
	r.Right = int64(math.Floor(cx + 0.5*float64(h)))
	r.Bottom = int64(math.Floor(cy + 0.5*float64(w)))
}

// Offset moves the rectangle by x and y.
func (r *Rectangle) Move(x, y int64) {
	r.Left += x
	r.Top += y
	r.Right += x
	r.Bottom += y
}

// Resize enlarges or shrinks r uniformly in all directions by an amount a,
// while keeping the center of r fixed.
func (r *Rectangle) Resize(a int64) {
	r.Left -= a
	r.Top -= a
	r.Right += a
	r.Bottom += a
}

// Overlap computes the overlapping area between r and s
func (r Rectangle) Overlap(s Rectangle) int64 {
	return (min(r.Right, s.Right) - max(r.Left, s.Left)) *
		(min(r.Bottom, s.Bottom) - max(r.Top, s.Top))
}

// OverlapPct computes the overlap area between r and s as a percentage of
// the total area covered by r and s, i.e. it computes:
//
//	P_ovr = A_ovr / (A_r + A_s - A_ovr)
func (r Rectangle) OverlapPct(s Rectangle) float64 {
	ovr := r.Overlap(s)
	return float64(ovr) / float64(r.Area()+s.Area()+ovr)
}

// Width returns the width of r.
func (r Rectangle) Width() int64 {
	return r.Right - r.Left
}

// String returns a string representation of r like "(3,4)-(6,5)".
func (r Rectangle) String() string {
	return fmt.Sprintf("(%d,%d)-(%d,%d)", r.Left, r.Top, r.Right, r.Bottom)
}
