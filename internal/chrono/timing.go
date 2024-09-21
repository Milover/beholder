package chrono

import (
	"fmt"
	"strings"
	"time"
)

type Timing struct {
	Name  string
	Value time.Duration
}

// Timings is a convenience type for easy handling of a set of timings.
type Timings []Timing

// Clear sets the length of ts to 0.
func (ts *Timings) Clear() {
	*ts = (*ts)[:0]
}

// Reset sets the duration of each [Timing] in ts to 0.
func (ts *Timings) Reset() {
	for i := range *ts {
		(*ts)[i].Value = time.Duration(0)
	}
}

// Set sets the duration value of the [Timing] name to val. If name is
// not present in ts, it is appended to ts with the value val.
func (ts *Timings) Set(name string, val time.Duration) {
	for i := range *ts {
		if (*ts)[i].Name == name {
			(*ts)[i].Value = val
			return
		}
	}
	*ts = append(*ts, Timing{Name: name, Value: val})
}

// String returns the formatted string representation of ts.
func (ts Timings) String() string {
	var b strings.Builder
	for i := range ts {
		fmt.Fprintf(&b, "%-15s: %v\n", ts[i].Name, ts[i].Value)
	}
	return b.String()
}
