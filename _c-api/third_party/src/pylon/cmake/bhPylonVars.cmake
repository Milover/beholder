# MODULE:   bhPylonVars
#
# PROVIDES:
#	bh_set_pylon_vars(
#		json-string
#		version-string
#		archive-var
#		url-var
#		md5-var
#	)
#
#	Extract pylon archive file name, download URL and MD5 hash from a
#	JSON configuration string for the requested pylon version.
#
#	The returned configuration variables are for the host platform.
#
#	The JSON file containing configuration data should have the following
#	structure:
#
#		[
#			{
#				"version": "0.1.2",
#				"vertag": "12345",
#				"linux": {
#					"amd64": {
#						"setup_archive": "pylon-0.1.2.12345_linux-x86_64_setup.tar.gz",
#						"archive": "pylon-0.1.2.12345_linux-x86_64.tar.gz",
#						"url_base": "https://example.com/download",
#						"md5": "8ce682ac8d9012b21c1533dc30ecc9de"
#					},
#					"arm64": { ... },
#					"some-other-arch": { ... }
#				},
#				"darwin": { ...  },
#				"some-other-platform": { ... }
#			},
#			{ ... }
#		]
#
# EXAMPLE USAGE:
#
#	project(beholder-pylon)
#
#	set(version "7.5.0")
#	file(READ "versions.json" json)
#
#	include(bhPylonVars.cmake)
#	bh_set_pylon_vars(
#		"${json}"
#		"${verson}"
#		archive-var
#		url-var
#		md5-var
#	)
#
#	ExternalProject_Add(
#		bh-pylon
#		URL "${url-var}"
#		URL_MD5 "${md5-var}"
#	)
#
#==============================================================================

function(bh_set_pylon_vars json ver archive url md5)
	unset(_ver)
	unset(_setup_archive)
	unset(_archive)
	unset(_url_base)
	unset(_md5)

	# set the os and architecture
	string(TOLOWER "${CMAKE_SYSTEM_NAME}" _os)
	string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" _arch)
	if("${_arch}" MATCHES "^(aarch64|arm64)")
		set(_arch "arm64")
	elseif("${_arch}" MATCHES "^(x86_64|arm64)")
		set(_arch "amd64")
	else()
		message(FATAL_ERROR "Unsupported pylon architecture: ${_arch}")
	endif()

	# find the config for the supplied version
	string(JSON _n_items LENGTH "${json}")
	math(EXPR _looprange "${_n_items} - 1")
	foreach(i RANGE ${_looprange})
		# get the current config and it's version
		string(JSON _item GET "${json}" ${i})
		string(JSON _ver GET "${_item}" "version")
		if ("${_ver}" STREQUAL "${ver}")
			# check if the ver-os-arch setup we want exists
			string(JSON _cfg ERROR_VARIABLE _err GET "${_item}" ${_os} ${_arch})
			if (NOT("${_err}" STREQUAL "NOTFOUND"))
				continue()
			endif()
			# pull out the config variables
			string(JSON _setup_archive GET "${_cfg}" "setup_archive")
			string(JSON _archive GET "${_cfg}" "archive")
			string(JSON _url_base GET "${_cfg}" "url_base")
			string(JSON _md5 GET "${_cfg}" "md5")
			# found our config, bail
			break()
		endif()
	endforeach()
	# check if config data has been correctly extracted
	if (NOT(_setup_archive AND _archive AND _url_base AND _md5))
		message(FATAL_ERROR "Could not find pylon config for ${_ver}-${_os}-${_arch} (version-os-arch)")
	endif()
	# use a local archive if it's available
	if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/archive/${_setup_archive}")
		set(_url_base "file://${CMAKE_CURRENT_SOURCE_DIR}/archive")
	endif()

	set(${archive} "${_archive}" PARENT_SCOPE)
	set(${url} "${_url_base}/${_setup_archive}" PARENT_SCOPE)
	set(${md5} "${_md5}" PARENT_SCOPE)
endfunction()
