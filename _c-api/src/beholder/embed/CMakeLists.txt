target_sources(beholder
	PRIVATE
		Binaries.cpp
		Embed.cpp
		Loader.cpp
		Manager.cpp
		Tar.cpp
	PUBLIC
		FILE_SET HEADERS
		FILES
			Embed.h
			Loader.h
			Manager.h
			Tar.h
)
