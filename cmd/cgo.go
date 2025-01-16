// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

package cmd

// Unfortunately this is necessary to ensure correct linkage, even though
// we don't actually use cgo directly anywhere in the package.
//
// This should be kept up to date with:
//	- internal/camera/cgo.go
//	- internal/imgproc/cgo.go
//	- internal/neural/cgo.go
//
// TODO: include/link directories should probably get loaded through
// environment variables.

/*
#cgo CXXFLAGS: -std=c++20
#cgo pkg-config: beholder_camera
#cgo CPPFLAGS: -I${SRCDIR}/../../c-deps/build/staging/include
#cgo linux LDFLAGS: -L${SRCDIR}/../../c-deps/build/staging/include/lib -lbeholder -ltesseract -lleptonica -lopencv_highgui -lopencv_videoio -lopencv_video -lopencv_imgcodecs -lopencv_dnn -lopencv_photo -lopencv_imgproc -lopencv_core -littnotify -lIlmImf -lippiw -lippicv -lade -llibjpeg-turbo -llibopenjp2 -llibpng -llibprotobuf -llibtiff -llibwebp -lzlib
*/
import "C"
