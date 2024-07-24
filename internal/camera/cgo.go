package camera

/*
#cgo CXXFLAGS: --std=c++17
#cgo CPPFLAGS: -I${SRCDIR}/_libcamera/include -I${SRCDIR}/_libcamera/share/include/pylon
#cgo linux LDFLAGS: -L${SRCDIR}/_libcamera/lib -L${SRCDIR}/_libcamera/share/lib -L${SRCDIR}/_libcamera/share/lib/pylon -Wl,-rpath,${SRCDIR}/_libcamera/lib:${SRCDIR}/_libcamera/share/lib/pylon -lcamera -lpylonc -lpylonutility -lpylonbase -lGenApi_gcc_v3_1_Basler_pylon -lGCBase_gcc_v3_1_Basler_pylon
*/
import "C"
