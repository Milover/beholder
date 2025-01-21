# CPack setup

function(bh_set_pkg_filename result)
	set(_bh_pkg ${PROJECT_NAME})
	set(_bh_ver ${PROJECT_VERSION})
	set(_bh_arch ${CMAKE_SYSTEM_PROCESSOR})
	set(_bh_os ${CMAKE_SYSTEM_NAME})
	set(_bh_comp ${CMAKE_CXX_COMPILER_ID})
	set(_bh_cfg ${CMAKE_BUILD_TYPE})

	string(TOLOWER 
		"${_bh_pkg}-${_bh_ver}-${_bh_arch}-${_bh_os}-${_bh_comp}-${_bh_cfg}"
		_bh_fname
	)
	set(${result} "${_bh_fname}" PARENT_SCOPE)
endfunction()

bh_set_pkg_filename(CPACK_PACKAGE_FILE_NAME)
set(CPACK_PACKAGE_CHECKSUM "MD5")
include(CPack)
