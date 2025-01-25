# MODULE:   bhPylonVars
#
# PROVIDES:
#	bh_set_pylon_vars(pylon-ver pylon-archive pylon-url pylon-md5)
#
#		Set up and export variables for pylon version, archive file name,
#		URL and archive MD5 hash.
#
#		FIXME: the URLs, MD5s and the version is hardcoded, so this will
#		probably break in the most inopportune time.
#
# EXAMPLE USAGE:
#
#	project(beholder-pylon)
#
#	include(bhPylonVars.cmake)
#	bh_set_pylon_vars(pylon-ver pylon-archive pylon-url pylon-md5)
#
#	ExternalProject_Add(
#		bh-pylon
#		URL "${pylon-url}"
#		URL_MD5 "${pylon-md5}"
#	)
#
#==============================================================================

function(bh_set_pylon_vars ver archive url md5)
	set(_p_pkg "pylon")
	set(_p_ver "8.0.2")
	set(_p_vertag "${_p_ver}.16314")
	string(TOLOWER "${CMAKE_SYSTEM_NAME}" _p_os)
	set(_p_arch "")
	if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "^(aarch64|arm64)")
		set(_p_arch "aarch64")
	else()
		set(_p_arch "${CMAKE_SYSTEM_PROCESSOR}")
	endif()
	set(_p_basename "${_p_pkg}-${_p_vertag}_${_p_os}-${_p_arch}")
	set(_p_setup "${_p_basename}_setup.tar.gz")
	set(_p_archive "${_p_basename}.tar.gz")

	set(_p_url "")
	set(_p_md5 "")
	if(_p_arch STREQUAL "x86_64")
		set(_p_url "https://downloadbsl.blob.core.windows.net/software/${_p_setup}")
		set(_p_md5 "8ce682ac8d9012b21c1533dc30ecc9de")
	elseif(_p_arch MATCHES "^(aarch64|arm64)")
		set(_p_url "https://downloads-ctf.baslerweb.com/dg51pdwahxgw/2hYtOdhN3XS2T8LXCD2PWm/4265334f5b42514b1d6c7ccc9c0d4b37/${_p_setup}")
		set(_p_md5 "f95e76bd5b6e839b097dec90d17e3e01")
	else()
		message(FATAL_ERROR "Unsupported ${_p_pkg} architecture: ${_p_arch}")
	endif()
	# use a local archive if it's available
	if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/archive/${_p_setup}")
		set(_p_url "file://${CMAKE_CURRENT_SOURCE_DIR}/archive/${_p_setup}")
	endif()

	set(${ver} "${_p_ver}" PARENT_SCOPE)
	set(${archive} "${_p_archive}" PARENT_SCOPE)
	set(${url} "${_p_url}" PARENT_SCOPE)
	set(${md5} "${_p_md5}" PARENT_SCOPE)
endfunction()
