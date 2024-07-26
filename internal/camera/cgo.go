package camera

/*
#cgo CXXFLAGS: --std=c++17
#cgo linux pkg-config: pylon
#cgo linux CPPFLAGS: -I/usr/local/include/camera
#cgo linux LDFLAGS: -L/usr/local/lib -lcamera
*/
import "C"
