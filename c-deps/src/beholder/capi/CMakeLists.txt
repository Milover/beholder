target_sources(beholder
	PRIVATE
		Result.cpp
	PUBLIC
		FILE_SET HEADERS
		FILES
			CWrapperBase.h
			RawImage.h
			Rectangle.h
			Result.h
)
