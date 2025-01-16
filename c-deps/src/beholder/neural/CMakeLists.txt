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
			BeholderNeural.h
			CRAFTDetector.h
			EASTDetector.h
			ObjDetector.h
			PARSeqDetector.h
			Tesseract.h
			YOLOv8Detector.h
)
