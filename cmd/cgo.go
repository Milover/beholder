package cmd

// Unfortunately this is necessary to ensure correct linkage, even though
// we don't actually use cgo directly anywhere in the package.
//
// This should be kept up to date with:
//	- internal/camera/cgo.go
//	- internal/image/cgo.go
//	- internal/ocr/cgo.go

/*
#cgo CXXFLAGS: -std=c++17
#cgo pkg-config: beholder_camera
#cgo CPPFLAGS: -I${SRCDIR}/../_libbeholder/share/include/beholder
#cgo LDFLAGS: -L${SRCDIR}/../_libbeholder/share/lib -L${SRCDIR}/../_libbeholder/share/lib/opencv4/3rdparty -lbeholder -ltesseract -lleptonica -lopencv_calib3d -lopencv_core -lopencv_dnn -lopencv_features2d -lopencv_flann -lopencv_gapi -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc -lopencv_ml -lopencv_objdetect -lopencv_photo -lopencv_stitching -lopencv_video -lopencv_videoio -littnotify -lIlmImf -lippiw -lippicv -lade -llibjpeg-turbo -llibopenjp2 -llibpng -llibprotobuf -llibtiff -llibwebp -lzlib -llapack -lblas
*/
import "C"
