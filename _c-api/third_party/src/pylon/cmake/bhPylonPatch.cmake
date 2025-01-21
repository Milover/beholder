# Patch pylon cmake files.

set(pylon_cmake_files
	"${bh_pylon_bindir}/share/pylon/cmake/genicam-targets.cmake"
	"${bh_pylon_bindir}/share/pylon/cmake/pylon-targets.cmake"
	"${bh_pylon_bindir}/share/pylon/cmake/pylon-targets-release.cmake"
)
foreach(file ${pylon_cmake_files})
	file(READ "${file}" pylon_cmfile)
	string(REPLACE
		[=["${_IMPORT_PREFIX}/include"]=]
		[=["${_IMPORT_PREFIX}/include/pylon"]=]
		pylon_cmfile "${pylon_cmfile}"
	)
	file(WRITE "${file}" "${pylon_cmfile}")
endforeach()
