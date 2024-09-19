// Package imgproc provides common image processing utilities.
package imgproc

// Changes here should be mirrored in internal/neural/cgo.go.

/*
#cgo CXXFLAGS: -std=c++17
#cgo CPPFLAGS: -I${SRCDIR}/../_libbeholder/include/beholder
#cgo darwin LDFLAGS: -L${SRCDIR}/../_libbeholder/lib -L${SRCDIR}/../_libbeholder/lib/opencv4/3rdparty -littnotify -lIlmImf -lade -llibjpeg-turbo -llibopenjp2 -llibpng -llibprotobuf -llibtiff -llibwebp -lzlib -lbeholder -ltesseract -lleptonica -lopencv_calib3d -lopencv_core -lopencv_dnn -lopencv_features2d -lopencv_flann -lopencv_gapi -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc -lopencv_ml -lopencv_objdetect -lopencv_photo -lopencv_stitching -lopencv_video -lopencv_videoio -llapack -lblas -framework AppKit -framework OpenCL
#cgo linux LDFLAGS: -L${SRCDIR}/../_libbeholder/lib -L${SRCDIR}/../_libbeholder/lib/opencv4/3rdparty -L/usr/local/cuda/lib64 -lbeholder -ltesseract -lleptonica -lopencv_core -lopencv_imgproc -lopencv_imgcodecs -lopencv_videoio -lopencv_highgui -lopencv_video -lopencv_dnn -lopencv_photo -lopencv_gapi -lopencv_cudev -lopencv_cudaimgproc -lopencv_cudaarithm -littnotify -lIlmImf -lippiw -lippicv -lade -llibjpeg-turbo -llibopenjp2 -llibpng -llibprotobuf -llibtiff -llibwebp -lzlib -lcudart -lcublas -lcudnn
*/
import "C"
