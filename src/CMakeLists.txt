# ocr - Copyright (C) 2024 P. Milovic
#
# See the LICENSE file for license information

# add subdirectories
#add_subdirectory(compute)

# add sources
set(SRC
	Ocr.cpp
	Preprocess.cpp
	Utility.cpp
	)

# append to sources
file(GLOB HDR RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.h)
append_ocr_src(${CMAKE_CURRENT_SOURCE_DIR} "${SRC};${HDR}")
