//go:build linux

package camera

import (
	"flag"
	"os"
	"testing"
)

// A hack to enable camera emulation during testing.
//
// Three emulated camera devices are made available for testing, since this
// should be enough for most use cases.
// The emulated camera devices have serial numbers of the form "0815-0xxx"
// where "xxx" is the zero-padded index of the emulated device. Indexing
// is zero-based, i.e. "0815-0001" is the serial number of the second
// emulated camera device.
//
// This variable is evaluated before any init() functions are run, and thus
// ensures that "PYLON_CAMEMU" is present in the environment before
// the pylon API is initialized.
var _ = func() (_ struct{}) {
	if err := os.Setenv("PYLON_CAMEMU", "3"); err != nil {
		panic("could not set 'PYLON_CAMEMU'")
	}
	return
}()

// Command line flags for the tests.
var (
	// serialNo is the serial number of the camera which is used for testing.
	// If it is not supplied as a command line flag, the one defined in
	// the test configuration is used.
	serialNo string
	// emuOnly determines whether to only run tests which use an emulated
	// camera device.
	// Only tests using an emulated camera device are run by default.
	emuOnly bool
	// hwTriggering determines whether to run tests which use
	// hardware triggering for image acquisition.
	// Hardware triggering tests are skipped by default.
	hwTriggering bool
	// nImgs determines how many image acquisition attempts should be made
	// for a test to complete.
	// Tests complete after 3 acquisition attempts by default.
	nImgs uint64
	// cleanUp determines whether to delete images written during a test
	// after the test completes.
	// Written images are deleted after a test completes by default.
	cleanUp bool
)

func init() {
	flag.StringVar(&serialNo, "sn", "", "serial number of camera used in tests")
	flag.BoolVar(&emuOnly, "emu-only", true, "only run tests using an emulated camera device")
	flag.BoolVar(&hwTriggering, "hw-trigger", false, "run tests which use hardware triggering")
	flag.Uint64Var(&nImgs, "n-img", 3, "number of image acquisition attempts for test completion")
	flag.BoolVar(&cleanUp, "cleanup", true, "delete images after a test completes")
}

// TestMain parses command line flags for the tests.
func TestMain(m *testing.M) {
	flag.Parse()
	code := m.Run()
	os.Exit(code)
}
