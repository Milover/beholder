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
			ScopeGuard.h
			Traits.h
			Units.h
			Utility.h
)
