target_sources(beholder
	PRIVATE
		Binaries.cpp
		Embed.cpp
		Manager.cpp
		Tar.cpp
	PUBLIC
		FILE_SET HEADERS
		FILES
			Embed.h
			Manager.h
			Tar.h
)
