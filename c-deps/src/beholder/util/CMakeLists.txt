target_sources(beholder
	PRIVATE
		Utility.cpp
	PUBLIC
		FILE_SET HEADERS
		FILES
			Constants.h
			Traits.h
			Utility.h
)
