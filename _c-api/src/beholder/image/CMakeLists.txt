add_subdirectory(ops)

target_sources(beholder
	PRIVATE
		ConversionInfo.cpp
		ProcessingOp.cpp
		Processor.cpp
	PUBLIC
		FILE_SET HEADERS
		FILES
			BeholderImage.h
			ConversionInfo.h
			ProcessingOp.h
			Processor.h
)
