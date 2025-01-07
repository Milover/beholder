cmake_minimum_required(VERSION 3.25.1)
project(beholder-pylon)

include(cmake/bhPylonVars.cmake)
bh_set_pylon_vars(pylon_ver pylon_archive pylon_url pylon_md5)

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
	UPDATE_DISCONNECTED TRUE
	PATCH_COMMAND
		${CMAKE_COMMAND}
			-Dbh_pylon_bindir=<BINARY_DIR>
			-P "${CMAKE_CURRENT_LIST_DIR}/cmake/bhPylonPatch.cmake"
	CONFIGURE_COMMAND ""
	BUILD_COMMAND ""
	INSTALL_COMMAND
		${CMAKE_COMMAND}
			-Dbh_pylon_projdir=${CMAKE_CURRENT_LIST_DIR}
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
