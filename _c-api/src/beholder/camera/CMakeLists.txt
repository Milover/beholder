target_sources(beholder
	PRIVATE
		Backends.cpp
		Camera.cpp
		CameraInterface.cpp
		Parameter.cpp
	PUBLIC
		FILE_SET HEADERS
		FILES
			Backends.h
			BeholderCamera.h
			Camera.h
			CameraInterface.h
			Config.h
			DeviceClass.h
			Parameter.h
			TriggerType.h
)

add_subdirectory(pylon)
