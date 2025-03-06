add_subdirectory(shim)

target_sources(beholder
	PRIVATE
		PylonCamera.cpp
	PUBLIC
		FILE_SET HEADERS
		FILES
			PylonCamera.h
)
