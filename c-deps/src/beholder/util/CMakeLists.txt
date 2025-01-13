target_sources(beholder
	PRIVATE
		Utility.cpp
	PUBLIC
		FILE_SET HEADERS
		FILES
			BeholderUtil.h
			Constants.h
			Enums.h
			Packs.h
			Traits.h
			Utility.h
)
