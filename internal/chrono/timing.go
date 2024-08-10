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

type Timings []Timing

func (ts *Timings) Reset() {
	for i := range *ts {
		(*ts)[i].Value = time.Duration(0)
	}
}

func (ts *Timings) Set(name string, val time.Duration) {
	for i := range *ts {
		if (*ts)[i].Name == name {
			(*ts)[i].Value = val
			return
		}
	}
	*ts = append(*ts, Timing{Name: name, Value: val})
}

func (ts Timings) String() string {
	var b strings.Builder
	for i := range ts {
		fmt.Fprintf(&b, "%-15s: %v\n", ts[i].Name, ts[i].Value)
	}
	return b.String()
}
