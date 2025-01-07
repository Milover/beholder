# MODULE:   bhGrabCli
#
# PROVIDES:
#	bh_grab_cli(args)
#
#		Grabs all CMake variables specified on the command line and stores
#		them into a variable so that they can be easily propagated (passed)
#		as CMAKE_ARGS to ExternalProject_Add calls.
#
#		NOTE: the function MUST be called before project() is called, because
#		project() sets/resets stuff.
#
# EXAMPLE USAGE:
#
#	include(bhGrabCli)
#	bh_grab_cli(args)
#
#	project(myProject)
#
#	ExternalProject_Add(
#		example
#		URL https://example.com
#		CMAKE_ARGS "${args}"
#	)
#
#==============================================================================

function(bh_grab_cli result)
	set(_bh_args ${_bh_args} CACHE STRING "comment")
	if (NOT _bh_args)
		get_cmake_property(_bh_vars CACHE_VARIABLES)
		foreach(_bh_var ${_bh_vars})
			get_property(_bh_help_str CACHE "${_bh_var}" PROPERTY HELPSTRING)
			if("${_bh_help_str}"
					MATCHES "No help, variable specified on the command line."
					OR "${_bh_help_str}" STREQUAL ""
			)
				# uncomment to see the variables being processed
				#message("${_bh_var} = [${${_bh_var}}]  --  ${_bh_help_str}")
				list(APPEND _bh_args "-D${_bh_var}=${${_bh_var}}")
			endif()
		endforeach()
	endif()

	set(${result} "${_bh_args}" PARENT_SCOPE)
endfunction()
