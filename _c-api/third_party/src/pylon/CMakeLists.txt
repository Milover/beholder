cmake_minimum_required(VERSION 3.25.1)
project(beholder-pylon)

# WARNING:
#
# v8.0.2
# ------
# - ASan reports an indirect memory leak in libpylon_TL_camemu.so
# - libpylon_TL_usb.so calls dlopen() with RTLD_DEEPBIND pylon-libusb-1.0.27.so
#	which makes ASan unusable;
#	see https://github.com/google/sanitizers/issues/611
#
# v7.5.0
# ------
# - ASan reports an indirect memory leak in libpylon_TL_camemu.so
# - libpylon_TL_usb.so calls dlopen() with RTLD_DEEPBIND pylon-libusb-1.0.so
#	which makes ASan unusable;
#	see https://github.com/google/sanitizers/issues/611

# the pylon version we're using (about ~500MB smaller, and we're not using
# any of the v8.0.2 features)
set(pylon_ver "7.5.0")

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/versions.json" pylon_json)

include(cmake/bhPylonVars.cmake)
bh_set_pylon_vars("${pylon_json}" "${pylon_ver}" pylon_archive pylon_url pylon_md5)

message(STATUS "pylon setup")
list(APPEND CMAKE_MESSAGE_INDENT "    ")
message(STATUS "version: ${pylon_ver}")
message(STATUS "archive: ${pylon_archive}")
message(STATUS "URL: ${pylon_url}")
message(STATUS "MD5: ${pylon_md5}")
list(POP_BACK CMAKE_MESSAGE_INDENT)

ExternalProject_Add(
	bh-pylon
	URL "${pylon_url}"
	URL_MD5 "${pylon_md5}"
	# 10 minute timeout
	TIMEOUT 600
	UPDATE_DISCONNECTED TRUE
	PATCH_COMMAND
		${CMAKE_COMMAND}
			-Dbh_pylon_bindir=<BINARY_DIR>
			-P "${CMAKE_CURRENT_LIST_DIR}/cmake/bhPylonPatch.cmake"
	CONFIGURE_COMMAND ""
	BUILD_COMMAND ""
	INSTALL_COMMAND
		${CMAKE_COMMAND}
			-Dbh_pylon_bindir=<BINARY_DIR>
			-Dbh_pylon_prefix=${bh_staging_prefix}
			-P "${CMAKE_CURRENT_LIST_DIR}/cmake/bhPylonInstall.cmake"
)

ExternalProject_Add_Step(
	bh-pylon unpack
	COMMAND
		${CMAKE_COMMAND}
			-Dbh_pylon_srcdir=<SOURCE_DIR>
			-Dbh_pylon_bindir=<BINARY_DIR>
			-Dbh_pylon_archive=${pylon_archive}
			-P "${CMAKE_CURRENT_LIST_DIR}/cmake/bhPylonUnpack.cmake"
	INDEPENDENT TRUE
	DEPENDEES download
	DEPENDERS patch
)

# configuring the pkg-config file from here makes things much simpler 
configure_file(
	"${CMAKE_CURRENT_LIST_DIR}/cmake/pylon.pc.in"
	"${bh_staging_prefix}/lib/pkgconfig/pylon.pc"
	@ONLY
)
