target_sources(beholder
	PRIVATE
		Errors.cpp
		Utility.cpp
	PUBLIC
		FILE_SET HEADERS
		FILES
			BeholderUtil.h
			Constants.h
			Enums.h
			Errors.h
			Packs.h
			ScopeGuard.h
			Traits.h
			Units.h
			Utility.h
)
