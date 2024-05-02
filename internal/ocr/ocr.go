package ocr

/*
#include <stdlib.h>
#include "ocr.h"
*/
import "C"
import (
	"errors"
	"strings"
	"unsafe"
)

type Config struct {
	p C.Cfg
}

func NewConfig(json string) *Config {
	cs := C.CString(json)
	defer C.free(unsafe.Pointer(cs))
	cfg := Config{p: C.Cfg_New(cs)}
	if cfg.p == nil {
		return nil
	}
	return &cfg
}

func (cfg Config) Delete() {
	C.Cfg_Delete(cfg.p)
}

type ImageProcessor struct {
	p C.Proc
}

func NewImageProcessor() *ImageProcessor {
	return &ImageProcessor{p: C.Proc_New()}
}

func (ip ImageProcessor) Delete() {
	C.Proc_Delete(ip.p)
}

func (ip ImageProcessor) Preprocess(c Config) error {
	if success := C.Proc_Preprocess(ip.p, c.p); !success {
		return errors.New("ocr.ImageProcessor.Preprocess: could not preprocess image")
	}
	return nil
}

func (ip ImageProcessor) ReadImage(filename string, flags int) error {
	cs := C.CString(filename)
	defer C.free(unsafe.Pointer(cs))
	if success := C.Proc_ReadImage(ip.p, cs, C.int(flags)); !success {
		return errors.New("ocr.ImageProcessor.ReadImage: could not read image")
	}
	return nil
}

func (ip ImageProcessor) ShowImage(title string) {
	cs := C.CString(title)
	defer C.free(unsafe.Pointer(cs))
	C.Proc_ShowImage(ip.p, cs)
}

type Tesseract struct {
	p C.Tess
}

func NewTesseract(cfg Config) *Tesseract {
	t := Tesseract{p: C.Tess_New(cfg.p)}
	if t.p == nil {
		return nil
	}
	return &t
}

func (t Tesseract) Delete() {
	C.Tess_Delete(t.p)
}

func (t Tesseract) Clear() {
	C.Tess_Clear(t.p)
}

func (t Tesseract) DetectText(ip ImageProcessor, cfg Config) error {
	if success := C.Tess_DetectText(t.p, ip.p, cfg.p); !success {
		return errors.New("ocr.Tesseract.DetectText: could not detect text")
	}
	return nil
}

func (t Tesseract) RecognizeText() string {
	ch := (*C.char)(C.Tess_RecognizeText(t.p))
	defer C.free(unsafe.Pointer(ch))
	return strings.TrimSpace(C.GoString(ch))
}

func (t Tesseract) SetImage(ip ImageProcessor, bytesPerPixel int) {
	C.Tess_SetImage(t.p, ip.p, C.int(bytesPerPixel))
}
