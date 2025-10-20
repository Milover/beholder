cmake_minimum_required(VERSION 3.25.1)
project(beholder-incbin)

# incbin is not a CMake project, however we want a CMake target.
# add_library is also not scriptable so we have to do it this way to force
# an installation.
ExternalProject_Add(
	bh-incbin
	URL "${CMAKE_CURRENT_SOURCE_DIR}/internal"
	INSTALL_DIR "${bh_staging_prefix}"
	CMAKE_ARGS
		-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
		-DCMAKE_PREFIX_PATH=<INSTALL_DIR>
)
