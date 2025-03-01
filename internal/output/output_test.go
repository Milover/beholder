// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

package output

import (
	"bytes"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"os"
	"testing"
	"time"

	"github.com/Milover/beholder/internal/models"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
)

var (
	resultTS        = time.Now()
	resultTSText, _ = resultTS.MarshalText()

	// result is the Result that is used in all tests.
	result = models.Result{
		Text:      []string{"some text", "some other text"},
		Expected:  []string{"some text", "some other text"},
		Status:    models.RSPass,
		Timestamp: resultTS,
	}

	resultCSV = fmt.Sprintf("timestamp,expected_l0,result_l0,expected_l1,result_l1,status\n%v,%s,%s,%s,%s,%v\n",
		string(resultTSText),
		result.Expected[0],
		result.Text[0],
		result.Expected[1],
		result.Text[1],
		result.Status,
	)

	resultJSONBytes, _ = json.Marshal(result)
	resultJSON         = string(resultJSONBytes) + "\n"
)

type outputerTest struct {
	Name         string
	ErrConstruct error
	ErrWrite     error
	ErrFlush     error
	ErrClose     error
	Typ          OutFmtType
	Expected     string
	// TypCheck asserts the correct type of the constructed io.WriteCloser.
	TypCheck func(Outputer)
}

var outputerTests = []outputerTest{
	{
		Name:         "bad-format",
		ErrConstruct: ErrBadOutputFormat,
		Typ:          OutFmtType(-1),
	},
	{
		Name:     "good-none",
		Typ:      OFTNone,
		TypCheck: func(o Outputer) { _ = o.(outNone) },
		Expected: "",
	},
	{
		Name:     "good-csv",
		Typ:      OFTCSV,
		TypCheck: func(o Outputer) { _ = o.(*outCSV) },
		Expected: resultCSV,
	},
	{
		Name:     "good-json",
		Typ:      OFTJSON,
		TypCheck: func(o Outputer) { _ = o.(outJSON) },
		Expected: resultJSON,
	},
}

// outputerTestTarget is a helper type, used as an output target by
// Outputers in tests.
type outputerTestTarget struct {
	bytes.Buffer
}

func (o *outputerTestTarget) Close() error {
	return nil
}

// TestOutputers tests whether Outputers output a correctly formatted Result.
func TestOutputers(t *testing.T) {
	for _, tt := range outputerTests {
		t.Run(tt.Name, func(t *testing.T) {
			assert := assert.New(t)
			require := require.New(t)

			var target outputerTestTarget

			o, err := newOutputer(tt.Typ, &target)
			require.ErrorIs(err, tt.ErrConstruct)

			// do further tests if o was successfully constructed
			if tt.ErrConstruct == nil {
				// check the constructed type
				assert.NotNil(tt.TypCheck, "TypCheck not defined")
				tt.TypCheck(o)

				// write data to target
				err := o.Write(&result)
				require.ErrorIs(err, tt.ErrWrite)

				// flush the buffer
				err = o.Flush()
				require.ErrorIs(err, tt.ErrFlush)

				// close the target
				err = o.Close()
				require.ErrorIs(err, errors.Join(tt.ErrFlush, tt.ErrClose))

				// check output
				assert.Equal(tt.Expected, target.String())
			}
		})
	}
}

type outTargetTest struct {
	Name         string
	ErrConstruct error
	ErrWrite     error
	ErrClose     error
	Typ          OutTargetType
	Spec         string
	// TypCheck asserts the correct type of the constructed io.WriteCloser.
	TypCheck func(io.WriteCloser)
	// Cleanup, if defined, will be registered as the test cleanup function
	// using t.Cleanup(f func()).
	Cleanup func(io.WriteCloser) func()
}

var outTargetTests = []outTargetTest{
	{
		Name:         "bad-target",
		ErrConstruct: ErrBadOutputTarget,
		Typ:          OutTargetType(-1),
		Spec:         `{}`,
	},
	{
		Name:     "good-none",
		Typ:      OTTNone,
		Spec:     `{}`,
		TypCheck: func(wc io.WriteCloser) { _ = wc.(outTargetNone) },
	},
	{
		Name:     "good-stdout",
		Typ:      OTTStdout,
		Spec:     `{}`,
		TypCheck: func(wc io.WriteCloser) { _ = wc.(outTargetStdout) },
	},
	{
		Name:     "good-file",
		Typ:      OTTFile,
		Spec:     `{"file": "definitely_does_not_exist"}`,
		TypCheck: func(wc io.WriteCloser) { _ = wc.(outTargetFile) },
		Cleanup: func(wc io.WriteCloser) func() {
			return func() {
				t := wc.(outTargetFile)
				if err := os.Remove(t.File); err != nil {
					panic(err)
				}
			}
		},
	},
	{
		Name:         "bad-empty-file",
		ErrConstruct: ErrOutputTargetEmptyFile,
		Typ:          OTTFile,
		Spec:         `{}`,
	},
}

func newOutTargetWRedirectedStdout(f func() (io.WriteCloser, error)) (io.WriteCloser, error) {
	orig := os.Stdout

	_, w, _ := os.Pipe()
	os.Stdout = w
	wc, err := f()
	os.Stdout = orig

	return wc, err
}

// TestOutTargets tests the operation of output targets set up
// from a JSON formatted configuration.
func TestOutTargets(t *testing.T) {
	for _, tt := range outTargetTests {
		t.Run(tt.Name, func(t *testing.T) {
			assert := assert.New(t)
			require := require.New(t)

			var m json.RawMessage
			err := json.Unmarshal([]byte(tt.Spec), &m)
			require.NoError(err, "could not unmarshal JSON spec")

			wc, err := newOutTargetWRedirectedStdout(func() (io.WriteCloser, error) {
				return newOutTarget(tt.Typ, m)
			})
			// optionally register the Cleanup function
			if tt.Cleanup != nil {
				t.Cleanup(tt.Cleanup(wc))
			}
			require.ErrorIs(err, tt.ErrConstruct)

			// do further tests if wc was successfully constructed
			if tt.ErrConstruct == nil {
				// check the constructed type
				assert.NotNil(tt.TypCheck, "TypCheck not defined")
				tt.TypCheck(wc)

				data := []byte("test")

				// write data to target
				count, err := wc.Write(data)
				assert.Equal(len(data), count, "mismatch in written bytes")
				require.ErrorIs(err, tt.ErrWrite)

				// close the target
				err = wc.Close()
				require.ErrorIs(err, tt.ErrClose)
			}
		})
	}
}
