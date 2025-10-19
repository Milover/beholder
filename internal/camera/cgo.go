// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

package camera

// NOTE: this needs to be kept up to date with:
//	- internal/camera/cgo.go
//	- internal/imgproc/cgo.go
//	- internal/neural/cgo.go

/*
#cgo CXXFLAGS: -std=c++20
#cgo LDFLAGS:-lbeholder -ltesseract -lleptonica -lopencv_videoio -lopencv_video -lopencv_imgcodecs -lopencv_dnn -lopencv_photo -lopencv_imgproc -lopencv_core -littnotify -lIlmImf -lippiw -lippicv -lade -llibjpeg-turbo -llibopenjp2 -llibpng -llibprotobuf -llibtiff -llibwebp -lz
*/
import "C"
