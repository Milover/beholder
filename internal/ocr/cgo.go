package ocr

/*
#cgo CXXFLAGS: --std=c++17
#cgo darwin CPPFLAGS: -I${SRCDIR}/_libocr/include
#cgo darwin LDFLAGS: -L${SRCDIR}/_libocr/lib -Wl,-rpath ${SRCDIR}/_libocr/lib -Wl,-rpath /usr/local/lib -locr
*/
import "C"
