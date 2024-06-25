package output

import (
	"bytes"
	"encoding/json"
	"fmt"
	"testing"
	"time"

	"github.com/Milover/ocr/internal/ocr"
	"github.com/stretchr/testify/assert"
)

var (
	resultTS        = time.Now()
	resultTSText, _ = resultTS.MarshalText()

	// result is the ocr.Result that is used in all tests.
	result = ocr.Result{
		Text:      "some text\nsome other text",
		Expected:  "some text\nsome other text",
		Status:    ocr.RSPass,
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

type formatTest struct {
	Name         string
	ErrConstruct error
	ErrWrite     error
	ErrClose     error
	Typ          FormatType
	Expected     string
	// TypCheck asserts the correct type of the constructed io.WriteCloser.
	TypCheck func(WriteCloser)
}

var formatTests = []formatTest{
	{
		Name:         "bad-format",
		ErrConstruct: ErrBadFormat,
		ErrWrite:     nil,
		ErrClose:     nil,
		Typ:          FormatType(-1),
	},
	{
		Name:         "good-none",
		ErrConstruct: nil,
		ErrWrite:     nil,
		ErrClose:     nil,
		Typ:          FTNone,
		TypCheck:     func(wc WriteCloser) { _ = wc.(ftNone) },
		Expected:     "",
	},
	{
		Name:         "good-csv",
		ErrConstruct: nil,
		ErrWrite:     nil,
		ErrClose:     nil,
		Typ:          FTCSV,
		TypCheck:     func(wc WriteCloser) { _ = wc.(*ftCSV) },
		Expected:     resultCSV,
	},
	{
		Name:         "good-json",
		ErrConstruct: nil,
		ErrWrite:     nil,
		ErrClose:     nil,
		Typ:          FTJSON,
		TypCheck:     func(wc WriteCloser) { _ = wc.(ftJSON) },
		Expected:     resultJSON,
	},
}

// ftWriteCloser is a helper type used as an output target by
// WriteClosers in tests.
type ftWriteCloser struct {
	bytes.Buffer
}

func (wc *ftWriteCloser) Close() error {
	return nil
}

// TestFormat tests if output formats output a correctly formatted ocr.Result.
func TestFormat(t *testing.T) {
	for _, tt := range formatTests {
		t.Run(tt.Name, func(t *testing.T) {
			assert := assert.New(t)

			var target ftWriteCloser

			wc, err := newFormat(tt.Typ, &target)
			assert.ErrorIs(err, tt.ErrConstruct)

			// do further tests if wc was succesfully constructed
			if tt.ErrConstruct == nil {
				// check the constructed type
				assert.NotNil(tt.TypCheck, "TypCheck not defined")
				tt.TypCheck(wc)

				// write data to target
				err := wc.Write(&result)
				assert.ErrorIs(err, tt.ErrWrite)

				// close the target
				err = wc.Close()
				assert.ErrorIs(err, tt.ErrClose)

				// check output
				assert.Equal(tt.Expected, target.String())
			}
		})
	}
}
