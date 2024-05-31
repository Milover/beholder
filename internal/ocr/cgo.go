package ocr

/*
#cgo CXXFLAGS: --std=c++17
#cgo CPPFLAGS: -I${SRCDIR}/_libocr/include
#cgo darwin LDFLAGS: -L${SRCDIR}/_libocr/lib -L${SRCDIR}/_libocr/share/lib -L${SRCDIR}/_libocr/share/lib/opencv4/3rdparty -lIlmImf -lade -littnotify -llibjpeg-turbo -llibopenjp2 -llibpng -llibprotobuf -llibtiff -llibwebp -ltegra_hal -lzlib -lopencv_calib3d -lopencv_core -lopencv_dnn -lopencv_features2d -lopencv_flann -lopencv_gapi -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc -lopencv_ml -lopencv_objdetect -lopencv_photo -lopencv_stitching -lopencv_video -lopencv_videoio -llapack -lblas -framework AppKit -framework OpenCL -lleptonica -ltesseract -locr
*/
import "C"
