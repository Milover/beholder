package ocr

/*
#cgo CXXFLAGS: --std=c++17
#cgo darwin pkg-config: opencv4 tesseract
#cgo darwin CPPFLAGS: -I${SRCDIR}/libocr/include
#cgo darwin LDFLAGS: -L${SRCDIR}/libocr/lib -Wl,-rpath ${SRCDIR}/libocr/lib -Wl,-rpath /usr/local/lib -locr
*/
import "C"
