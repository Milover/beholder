package neural

// Changes here should be mirrored in internal/imgproc/cgo.go.
//
// TODO: include/link directories should probably get loaded through
// environment variables.

/*
#cgo CXXFLAGS: -std=c++20
#cgo CPPFLAGS: -I${SRCDIR}/../../c-deps/build/staging/include
#cgo linux LDFLAGS: -L${SRCDIR}/../../c-deps/build/staging/include/lib -lbeholder -ltesseract -lleptonica -lopencv_highgui -lopencv_videoio -lopencv_video -lopencv_imgcodecs -lopencv_dnn -lopencv_photo -lopencv_imgproc -lopencv_core -littnotify -lIlmImf -lippiw -lippicv -lade -llibjpeg-turbo -llibopenjp2 -llibpng -llibprotobuf -llibtiff -llibwebp -lzlib
*/
import "C"
