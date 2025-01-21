# Install pylon package files (to staging dir).

file(INSTALL "${bh_pylon_bindir}/include/"
	DESTINATION "${bh_pylon_prefix}/include/pylon"
	FILES_MATCHING PATTERN "*"
)
file(INSTALL
	"${bh_pylon_bindir}/lib/libpylonbase.so"
	"${bh_pylon_bindir}/lib/libpylonc.so"
	"${bh_pylon_bindir}/lib/libpylonutility.so"
	"${bh_pylon_bindir}/lib/libFirmwareUpdate_gcc_v3_1_Basler_pylon.so"
	"${bh_pylon_bindir}/lib/libGCBase_gcc_v3_1_Basler_pylon.so"
	"${bh_pylon_bindir}/lib/libGenApi_gcc_v3_1_Basler_pylon.so"
	"${bh_pylon_bindir}/lib/libLog_gcc_v3_1_Basler_pylon.so"
	"${bh_pylon_bindir}/lib/liblog4cpp_gcc_v3_1_Basler_pylon.so"
	"${bh_pylon_bindir}/lib/libMathParser_gcc_v3_1_Basler_pylon.so"
	"${bh_pylon_bindir}/lib/libNodeMapData_gcc_v3_1_Basler_pylon.so"
	"${bh_pylon_bindir}/lib/libXmlParser_gcc_v3_1_Basler_pylon.so"
	"${bh_pylon_bindir}/lib/libgxapi.so"
	"${bh_pylon_bindir}/lib/libuxapi.so"
	"${bh_pylon_bindir}/lib/libpylon_TL_gige.so"
	"${bh_pylon_bindir}/lib/libpylon_TL_camemu.so"
	"${bh_pylon_bindir}/lib/libpylon_TL_usb.so"
	"${bh_pylon_bindir}/lib/pylon-libusb-1.0.27.so"
	DESTINATION "${bh_pylon_prefix}/lib"
	FOLLOW_SYMLINK_CHAIN
)
file(INSTALL "${bh_pylon_bindir}/share/pylon/cmake/"
	DESTINATION "${bh_pylon_prefix}/lib/cmake/pylon"
	FILES_MATCHING PATTERN "*.cmake"
)
file(INSTALL "${bh_pylon_bindir}/share/pylon/licenses/"
	DESTINATION "${bh_pylon_prefix}/share/licenses/pylon"
	FILES_MATCHING PATTERN "*.txt"
)
