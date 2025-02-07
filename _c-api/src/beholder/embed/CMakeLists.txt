target_sources(beholder
	PRIVATE
		Embed.cpp
		Binaries.cpp
		Tar.cpp
	PUBLIC
		FILE_SET HEADERS
		FILES
			Embed.h
			Tar.h
)
