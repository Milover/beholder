package neural

/*
#include <stdlib.h>
#include "neural.h"
*/
import "C"
import (
	"errors"
	"path"
	"slices"
	"unsafe"

	"github.com/Milover/beholder/internal/enumutils"
	"github.com/Milover/beholder/internal/mem"
	"github.com/Milover/beholder/internal/models"
	"github.com/Milover/beholder/internal/neural/model"
)

var (
	ErrInference = errors.New("inference error")      // [Network.Inference] error
	ErrInit      = errors.New("initialization error") // [Network.Init] error
	ErrAPIPtr    = errors.New("nil API pointer")      // uninitialized C-API error
)

// Network is a generic neural network used for image processing.
//
// WARNING: a Network usually contains C-managed resources, hence, when
// it is no longer needed, [Network.Delete] must be called to correctly
// free the resources and clean up.
//
// TODO: move Init and Delete to a C-API interface.
type Network interface {
	// Clear clears results held internally by the C-API.
	// The Network should still be valid and initialized after calling Clear.
	Clear()
	// Delete releases C-allocated memory.
	// Once called, the Network is no longer valid.
	Delete()
	// Inference performs inferencing on the supplied image.
	// Before calling Inference, the Network must be initialized by
	// calling [Network.Init].
	//
	// Internally stored results are cleared by the C-API when
	// Inference is called.
	Inference(models.Image, *models.Result) error
	// Init initializes the Network (C-allocated API) with configuration data.
	Init() error
	// TODO: do we need a Config() call?
}

// Backend is a [Network] computation backends. See the [OpenCV docs] for
// more info.
//
// Note that OpenCV needs to be compiled with additional backend support
// for most of these to be usable (eg. CUDA/CUDNN, OpenVINO...)
//
// [OpenCV docs]: https://docs.opencv.org/4.10.0/d6/d0f/group__dnn.html#ga186f7d9bfacac8b0ff2e26e2eab02625
type Backend int

// String returns a string representation of b.
func (b Backend) String() string {
	s, ok := backendMap[b]
	if !ok {
		return "unknown"
	}
	return s
}

// UnmarshallJSON unmarshals d from JSON.
func (b *Backend) UnmarshalJSON(data []byte) error {
	return enumutils.UnmarshalJSON(data, b, invBackendMap)
}

// MarshallJSON marshals d into JSON.
func (b Backend) MarshalJSON() ([]byte, error) {
	return enumutils.MarshalJSON(b, backendMap)
}

const (
	// FIXME: yolo
	BackendDefault  Backend = iota // OpenCV default backend (usually OpenCV)
	BackendHalide                  // Halide backend
	BackendOpenVINO                // OpenVINO backend
	BackendOpenCV                  // OpenCV backend
	BackendVulkan                  // Vulkan backend
	BackendCUDA                    // CUDA backend
	BackendWebNN                   // Microsoft WebNN
	BackendTIMVX                   // VeriSilicon TIM-VX
	BackendCANN                    // Huawei CANN backend
)

var (
	backendMap = map[Backend]string{
		BackendDefault:  "default",
		BackendHalide:   "halide",
		BackendOpenVINO: "openvino",
		BackendOpenCV:   "opencv",
		BackendVulkan:   "vulkan",
		BackendCUDA:     "cuda",
		BackendWebNN:    "webnn",
		BackendTIMVX:    "timvx",
		BackendCANN:     "cann",
	}
	invBackendMap = enumutils.Invert(backendMap)
)

// Target is the device used by the [Network] for computation. See the
// [OpenCV docs] for more info.
//
// [OpenCV docs]: https://docs.opencv.org/4.10.0/d6/d0f/group__dnn.html#ga709af7692ba29788182cf573531b0ff5
type Target int

// String returns a string representation of t.
func (t Target) String() string {
	s, ok := targetMap[t]
	if !ok {
		return "unknown"
	}
	return s
}

// UnmarshallJSON unmarshals d from JSON.
func (t *Target) UnmarshalJSON(data []byte) error {
	return enumutils.UnmarshalJSON(data, t, invTargetMap)
}

// MarshallJSON marshals d into JSON.
func (t Target) MarshalJSON() ([]byte, error) {
	return enumutils.MarshalJSON(t, targetMap)
}

const (
	// FIXME: yolo
	TargetCPU        Target = iota // CPU
	TargetOpenCL                   // OpenCL
	TargetOpenCLFP16               // mixed precision OpenCL
	TargetMyriad                   // Xailient AI Myriad 2/X
	TargetVulkan                   // Vulkan GPU
	TargetFPGA                     // FPGA
	TargetCUDA                     // CUDA GPU
	TargetCUDAFP16                 // mixed precision CUDA GPU
	TargetHDDL                     // OpenVINO HDDL
	TargetNPU                      // NPU
	TargetCPUFP16                  // mixed-precision CPU, ARM only
)

var (
	targetMap = map[Target]string{
		TargetCPU:        "cpu",
		TargetOpenCL:     "opencl",
		TargetOpenCLFP16: "opencl-fp16",
		TargetMyriad:     "myriad",
		TargetVulkan:     "vulkan",
		TargetFPGA:       "fpga",
		TargetCUDA:       "cuda",
		TargetCUDAFP16:   "cuda-fp16",
		TargetHDDL:       "hddl",
		TargetNPU:        "npu",
		TargetCPUFP16:    "cpu-fp16",
	}
	invTargetMap = enumutils.Invert(targetMap)
)

// network is a helper type which implements the [Network] interface.
//
// It is usually embedded into other concrete types since most implementations
// have similar functionality and requirements, thus making implementing new
// NN models fairly painless.
//
// WARNING: network contains C-managed resources so when it is no longer needed,
// [network.Delete] must be called to release the resources and clean up.
type network struct {
	// Backend is the NN computation backend.
	Backend Backend `json:"backend"`
	// Target is the NN computation device.
	Target Target `json:"target"`

	// Model is the NN model definition handle.
	// It can either be an embedded model keyword, or a model file path.
	Model model.Model `json:"model"`
	// Config is the network configuration.
	Config *Config `json:"config"`

	p C.Det // pointer to the C++ API class.
}

// newNetwork constructs (C call) a new uninitialized network.
// WARNING: [network.Delete] must be called to release the memory
// when no longer needed.
func newNetwork() network {
	return network{
		Backend: BackendDefault,
		Target:  TargetCPU,
		Config:  NewConfig(),
	}
}

// Clear clears results held internally by the C-API.
// Note that n is still valid and initialized after calling Clear.
func (n network) Clear() {
	C.Det_Clear(n.p)
}

// Delete releases C-allocated memory. Once called, n is no longer valid.
func (n *network) Delete() {
	C.Det_Delete(n.p)
}

// Inference performs inferencing on the supplied image.
// Before calling Inference, n must be initialized by calling [network.Init].
//
// Internally stored results are cleared by the C-API when Inference is called.
func (n network) Inference(img models.Image, res *models.Result) error {
	ar := &mem.Arena{}
	defer ar.Free()

	raw := toCImg(img)
	results := (*C.ResArr)(ar.Store(
		unsafe.Pointer(C.Det_Detect(n.p, &raw)),
		unsafe.Pointer(C.ResArr_Delete)))
	if unsafe.Pointer(results) == nil {
		return ErrInference
	}
	fromCRes(results, res)
	return nil
}

// Init initializes the C-allocated API with the configuration data,
// if n is valid.
func (n network) Init() error {
	if err := n.IsValid(); err != nil {
		return err
	}
	// handle the model
	mfn, cleanup, err := n.Model.File()
	if err != nil {
		return err
	}
	defer cleanup() // this could fail, but we don't care

	ar := &mem.Arena{}
	defer ar.Free()

	// allocate the struct and handle the easy stuff (ints, strings...)
	in := C.DetInit{
		modelPath: (*C.char)(ar.CopyStr(path.Dir(mfn))),
		model:     (*C.char)(ar.CopyStr(path.Base(mfn))),
		backend:   C.int(n.Backend),
		target:    C.int(n.Target),
		conf:      C.float(n.Config.ConfidenceThreshold),
		nms:       C.float(n.Config.NMSThreshold),
		swapRB:    C.bool(n.Config.SwapRB),
	}

	// assign arrays
	v2Asgn := func(from [2]int, to *[2]C.int) {
		for i := range 2 {
			to[i] = C.int(from[i])
		}
	}
	v3Asgn := func(from [3]float64, to *[3]C.double) {
		for i := range 3 {
			to[i] = C.double(from[i])
		}
	}
	v2Asgn(n.Config.Size, &in.size)
	v3Asgn(n.Config.Scale, &in.scale)
	v3Asgn(n.Config.Mean, &in.mean)
	v3Asgn(n.Config.PadValue, &in.pad)

	if ok := C.Det_Init(n.p, &in); !ok {
		return ErrInit
	}
	return nil
}

// IsValid asserts that n can be initialized.
func (n network) IsValid() error {
	if n.p == (C.Det)(nil) {
		return ErrAPIPtr
	}
	if err := n.Config.IsValid(); err != nil {
		return err
	}
	return nil
}

// toCImg returns a copy of img as a C-image.
func toCImg(img models.Image) C.Img {
	return C.Img{
		C.size_t(img.ID),
		C.int(img.Rows),
		C.int(img.Cols),
		C.int64_t(img.PixelType),
		img.Buffer,
		C.size_t(img.Step),
		C.size_t(img.BitsPerPixel),
	}
}

// fromCRes copies a C-Result array into res.
func fromCRes(cRes *C.ResArr, res *models.Result) {
	// allocate and reset if necessary
	nLines := uint64(cRes.count)
	if uint64(cap(res.Confidences)) < nLines {
		diff := int(nLines - uint64(cap(res.Confidences)))
		res.Text = slices.Grow(res.Text, diff)
		res.Confidences = slices.Grow(res.Confidences, diff)
		res.Angles = slices.Grow(res.Angles, diff)
		res.Boxes = slices.Grow(res.Boxes, diff)
	}
	// FIXME: we probably shouldn't do this here
	res.Text = res.Text[:0]
	res.Confidences = res.Confidences[:0]
	res.Angles = res.Angles[:0]
	res.Boxes = res.Boxes[:0]
	// populate the result
	resultsSl := unsafe.Slice(cRes.array, nLines)
	// FIXME: the C-Result should only contain fields which the network can
	// actually generate, i.e. a text detector shouldn't need to have a 'text'
	// field, and a text recognizer shouldn't need to have a 'box' field.
	for _, r := range resultsSl {
		res.Text = append(res.Text, C.GoString(r.text))
		res.Confidences = append(res.Confidences, float64(r.confidence))
		res.Angles = append(res.Angles, float64(r.boxRotAngle))
		res.Boxes = append(res.Boxes, models.Rectangle{
			Left:   int64(r.box.left),
			Top:    int64(r.box.top),
			Right:  int64(r.box.right),
			Bottom: int64(r.box.bottom),
		})
	}
}
