target_sources(beholder
	PRIVATE
		Binaries.cpp
		Embed.cpp
		Loader.cpp
		Tar.cpp
		Unpacker.cpp
	PUBLIC
		FILE_SET HEADERS
		FILES
			Embed.h
			Loader.h
			Tar.h
			Unpacker.h
)
