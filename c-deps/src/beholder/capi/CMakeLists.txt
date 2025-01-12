target_sources(beholder
	PRIVATE
		Result.cpp
	PUBLIC
		FILE_SET HEADERS
		FILES
			Image.h
			Rectangle.h
			Result.h
			Wrapper.h
)
