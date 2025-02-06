target_sources(beholder
	PRIVATE
		Embed.cpp
		Binaries.cpp
	PUBLIC
		FILE_SET HEADERS
		FILES
			Embed.h
			Tar.h
)
