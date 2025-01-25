# Update pkg-config paths.

if(NOT EXISTS ${bh_pkgcfg_path})
	message(FATAL_ERROR "pkg-config file not found: ${bh_pkgcfg_path}")
endif()

# update prefix path
file(READ "${bh_pkgcfg_path}" bh_pkgcfg_str)
string(REPLACE "${bh_staging_prefix}" "${CMAKE_INSTALL_PREFIX}" bh_pkgcfg_str "${bh_pkgcfg_str}")
file(WRITE "${bh_pkgcfg_path}" "${bh_pkgcfg_str}")

message(STATUS "Updated ${bh_pkgcfg_path} - from ${bh_staging_prefix} to ${CMAKE_INSTALL_PREFIX}")
