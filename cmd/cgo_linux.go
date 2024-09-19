package cmd

// Unfortunately this is necessary to ensure correct linkage, even though
// we don't actually use cgo directly anywhere in the package.
//
// This should be kept up to date with:
//	- internal/camera/cgo.go
//	- internal/imgproc/cgo.go
//	- internal/neural/cgo.go

/*
#cgo CXXFLAGS: -std=c++17
#cgo pkg-config: beholder_camera
#cgo CPPFLAGS: -I${SRCDIR}/../_libbeholder/include/beholder
#cgo linux LDFLAGS: -L${SRCDIR}/../_libbeholder/lib -L${SRCDIR}/../_libbeholder/lib/opencv4/3rdparty -L/usr/local/cuda/lib64 -lbeholder -ltesseract -lleptonica -lopencv_core -lopencv_imgproc -lopencv_imgcodecs -lopencv_videoio -lopencv_highgui -lopencv_video -lopencv_dnn -lopencv_photo -lopencv_gapi -lopencv_cudev -lopencv_cudaimgproc -lopencv_cudaarithm -littnotify -lIlmImf -lippiw -lippicv -lade -llibjpeg-turbo -llibopenjp2 -llibpng -llibprotobuf -llibtiff -llibwebp -lzlib -lcudart -lcublas -lcudnn
*/
import "C"
