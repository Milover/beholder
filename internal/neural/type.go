// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

package neural

/*
#include <stdlib.h>
#include "neural.h"
*/
import "C"
import (
	"errors"
	"fmt"

	"github.com/Milover/beholder/internal/enumutils"
)

var (
	ErrType = errors.New("bad network type") // bad NN model (network) type
)

// Type is the type of NN model a [Network] uses, i.e. the network type.
type Type int

// TypeUnknown is the default [Type] value and denotes an unknown network type.
const TypeUnknown Type = 0

var (
	typeMap    = map[Type]string{TypeUnknown: "unknown"}
	invTypeMap = enumutils.Invert(typeMap)
)

// String returns a string representation of t.
func (t Type) String() string {
	s, ok := typeMap[t]
	if !ok {
		return typeMap[TypeUnknown]
	}
	return s
}

// UnmarshallJSON unmarshals t from JSON.
func (t *Type) UnmarshalJSON(data []byte) error {
	return enumutils.UnmarshalJSON(data, t, invTypeMap)
}

// MarshallJSON marshals t into JSON.
func (t Type) MarshalJSON() ([]byte, error) {
	return enumutils.MarshalJSON(t, typeMap)
}

// Factory creates a new [Network] with a default configuration.
type Factory func() Network

// networkFactoryMap maps a [Type] to a [Factory].
//
// When a new NN model is implemented, it should be registered with
// [RegisterNetworkType].
var networkFactoryMap = make(map[Type]Factory)

// RegisterNetwork registers a new [Network] by associating it's [Type] with
// a keyword and a [Factory] function.
//
// Panics if either typ or keyword have already been registered.
//
// FIXME: when a new [Type] is defined somewhere else, how should the user
// know which value to choose for the type, i.e. whether the value is unique?
func RegisterNetwork(typ Type, keyword string, fn Factory) {
	_, foundTM := typeMap[typ]
	_, foundNFM := networkFactoryMap[typ]
	if foundTM || foundNFM {
		panic("tried to register an existing network type")
	}
	if _, found := invTypeMap[keyword]; found {
		panic("tried to register an existing network keyword")
	}
	typeMap[typ] = keyword
	invTypeMap[keyword] = typ
	networkFactoryMap[typ] = fn
}

// NewNetwork creates and returns a new [Network] with a default configuration.
func NewNetwork(t Type) (Network, error) {
	f, ok := networkFactoryMap[t]
	if !ok {
		return nil, fmt.Errorf("neural.GetNetwork: %w", ErrType)
	}
	return f(), nil
}
