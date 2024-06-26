package ocr

import (
	"bytes"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"os"
	"testing"
	"time"

	"github.com/stretchr/testify/assert"
)

var (
	resultTS        = time.Now()
	resultTSText, _ = resultTS.MarshalText()

	// result is the Result that is used in all tests.
	result = Result{
		Text:      "some text\nsome other text",
		Expected:  "some text\nsome other text",
		Status:    RSPass,
		TimeStamp: resultTS,
	}

	resultCSV = fmt.Sprintf("timestamp,expected,result,status\n%v,\"%v\",\"%v\",%v\n",
		string(resultTSText),
		result.Expected,
		result.Text,
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

			var target outputerTestTarget

			o, err := newOutputer(tt.Typ, &target)
			assert.ErrorIs(err, tt.ErrConstruct)

			// do further tests if o was succesfully constructed
			if tt.ErrConstruct == nil {
				// check the constructed type
				assert.NotNil(tt.TypCheck, "TypCheck not defined")
				tt.TypCheck(o)

				// write data to target
				err := o.Write(&result)
				assert.ErrorIs(err, tt.ErrWrite)

				// flush the buffer
				err = o.Flush()
				assert.ErrorIs(err, tt.ErrFlush)

				// close the target
				err = o.Close()
				assert.ErrorIs(err, errors.Join(tt.ErrFlush, tt.ErrClose))

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

// TestOutTargets tests the operation of output targets set up
// from a JSON formatted configuration.
func TestOutTargets(t *testing.T) {
	for _, tt := range outTargetTests {
		t.Run(tt.Name, func(t *testing.T) {
			assert := assert.New(t)

			var m json.RawMessage
			err := json.Unmarshal([]byte(tt.Spec), &m)
			assert.Nil(err, "could not unmarshal JSON spec")

			wc, err := newOutTarget(tt.Typ, m)
			// optionally register the Cleanup function
			if tt.Cleanup != nil {
				t.Cleanup(tt.Cleanup(wc))
			}
			assert.ErrorIs(err, tt.ErrConstruct)

			// do further tests if wc was succesfully constructed
			if tt.ErrConstruct == nil {
				// check the constructed type
				assert.NotNil(tt.TypCheck, "TypCheck not defined")
				tt.TypCheck(wc)

				data := []byte("test")

				// write data to target
				count, err := wc.Write(data)
				assert.Equal(len(data), count, "mismatch in written bytes")
				assert.ErrorIs(err, tt.ErrWrite)

				// close the target
				err = wc.Close()
				assert.ErrorIs(err, tt.ErrClose)
			}
		})
	}
}
