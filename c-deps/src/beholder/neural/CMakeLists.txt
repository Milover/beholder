target_compile_definitions(beholder
	PRIVATE
		CV_DNN_DONT_ADD_INLINE_NS
)

add_subdirectory(internal)

target_sources(beholder
	PRIVATE
		CRAFTDetector.cpp
		EASTDetector.cpp
		ObjDetector.cpp
		PARSeqDetector.cpp
		Tesseract.cpp
		YOLOv8Detector.cpp
	PUBLIC
		FILE_SET HEADERS
		FILES
			CRAFTDetector.h
			EASTDetector.h
			ObjDetector.h
			PARSeqDetector.h
			Tesseract.h
			YOLOv8Detector.h
)
