target_sources(beholder_camera
	PRIVATE
		DefaultConfigurator.cpp
	PRIVATE
		FILE_SET internal
		TYPE HEADERS
		FILES
			DefaultConfigurator.h
)
