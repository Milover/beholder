package cmd

import (
	"fmt"
	"os"
	"path"
	"reflect"
	"strconv"
	"time"
	"unsafe"
)

// id is a helper type, so that Filename can receive an arbitrary string.
type id struct {
	ID string // an arbitrary string ID
}

// getter is function that retrieves a value from a type as a formatted string.
type getter func(unsafe.Pointer) any

func createGetter(typ reflect.Type, fldName string) (getter, error) {
	fld, ok := typ.FieldByName(fldName)
	if !ok {
		return nil, fmt.Errorf("invalid field name: %q", fldName)
	}
	switch fld.Type.Kind() {
	case reflect.Bool:
		return func(ptr unsafe.Pointer) any {
			return strconv.FormatBool(*(*bool)(unsafe.Add(ptr, fld.Offset)))
		}, nil
	case reflect.Int64:
		return func(ptr unsafe.Pointer) any {
			return strconv.FormatInt(*(*int64)(unsafe.Add(ptr, fld.Offset)), 10)
		}, nil
	case reflect.Uint64:
		return func(ptr unsafe.Pointer) any {
			return strconv.FormatUint(*(*uint64)(unsafe.Add(ptr, fld.Offset)), 10)
		}, nil
	case reflect.Float64:
		return func(ptr unsafe.Pointer) any {
			return strconv.FormatFloat(*(*float64)(unsafe.Add(ptr, fld.Offset)), 'g', -1, 64)
		}, nil
	case reflect.String:
		return func(ptr unsafe.Pointer) any {
			return *(*string)(unsafe.Add(ptr, fld.Offset))
		}, nil
	case reflect.Struct:
		switch {
		case fld.Type == reflect.TypeOf(time.Time{}):
			return func(ptr unsafe.Pointer) any {
				return ((*time.Time)(unsafe.Add(ptr, fld.Offset))).Format("2006-01-02_15-04-05")
			}, nil
		default:
			break
		}
	}
	return nil, fmt.Errorf("field %q is of unsupported type %q", fldName, fld.Type)
}

// TODO: move this to an appropriate package.
// Filename is a runtime configurable file name generator.
type Filename[T any] struct {
	// FString is a Go format string used to generate the file name.
	// It can be either a relative or an absolute path, as long as it
	// is valid, ie. the parent directory exists.
	FString string `json:"f_string"`
	// Fields are the names of fields used as input to FString.
	// Ordering is preserved when passed to FString.
	Fields []string `json:"fields"`

	// getters are getter functions used for retrieving field values from
	// some value of type 'typ'.
	getters []getter
}

// Init initializes the file name generator using 't'.
func (f *Filename[T]) Init(t T) error {
	typ := reflect.TypeOf(t)
	for _, field := range f.Fields {
		g, err := createGetter(typ, field)
		if err != nil {
			return err
		}
		f.getters = append(f.getters, g)
	}
	return nil
}

// Get generates a filename using field values of 't'.
func (f Filename[T]) Get(t *T) (string, error) {
	vals := make([]any, 0, len(f.getters))
	for _, g := range f.getters {
		vals = append(vals, g(unsafe.Pointer(t)))
	}
	filename := fmt.Sprintf(f.FString, vals...)
	if _, err := os.Stat(path.Dir(filename)); err != nil {
		return filename, err
	}
	return filename, nil
}
