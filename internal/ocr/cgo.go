package ocr

/*
#cgo CXXFLAGS: --std=c++17
#cgo linux pkg-config: opencv4 tesseract lept
#cgo linux CPPFLAGS: -I/usr/local/include/ocr
#cgo linux LDFLAGS: -L/usr/local/lib -locr
*/
import "C"
