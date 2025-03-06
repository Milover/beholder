target_sources(beholder
	PRIVATE
		Backends.cpp
		Camera.cpp
		Parameter.cpp
		TransportLayer.cpp
	PUBLIC
		FILE_SET HEADERS
		FILES
			Backends.h
			Camera.h
			Parameter.h
			TransportLayer.h
)

add_subdirectory(pylon)
