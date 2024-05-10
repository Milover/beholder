#include <cstring>
#include <vector>
#include <string>

#include "ocr.h"


void Proc_Delete(Proc p) {
	if (p) {
		delete p;
		p = nullptr;
	}
}

bool Proc_Init(Proc p, void** post, size_t nPost, void** pre, size_t nPre) {
	if (!p) {
		return false;
	}
	auto helper = []
	(
		ocr::ImageProcessor::OpList& list,
		void** ptrs,
		std::size_t nPtrs
	) -> void
	{
		list.reserve(nPtrs);
		for (auto i {0ul}; i < nPtrs; ++i) {
			list.emplace_back(static_cast<ocr::ProcessingOp*>(ptrs[i]));
		}
	};
	helper(p->postprocessing, post, nPost);
	helper(p->preprocessing, pre, nPre);
	return true;
}

Proc Proc_New() {
	return new ocr::ImageProcessor {};
}

bool Proc_Postprocess(Proc p, Tess t) {
	if (!p || !t) {
		return false;
	}
	return p->postprocess(t->getResults());
}

bool Proc_Preprocess(Proc p) {
	if (!p) {
		return false;
	}
	return p->preprocess();
}

bool Proc_ReadImage(Proc p, const char* filename, int flags) {
	if (!p) {
		return false;
	}
	std::string s {filename};
	return p->readImage(s, flags);
}

void Proc_ShowImage(Proc p, const char* title) {
	if (!p) {
		return;
	}
	std::string s {title};
	p->showImage(s);
}

void Tess_Clear(Tess t) {
	if (t) {
		t->clear();
	}
}

void Tess_Delete(Tess t) {
	if (t) {
		delete t;
		t = nullptr;
	}
}

char* Tess_DetectAndRecognize(Tess t) {
	if (!t) {
		return nullptr;
	}
	if (!t->detectText()) {
		return nullptr;
	}
	if (!t->recognizeText()) {
		return nullptr;
	}
	const std::string& txt {t->getResults().text};
	char* result {new char[txt.length() + 1]};
	std::strncpy(result, txt.c_str(), txt.length() + 1);
	return result;
}

bool Tess_Init(Tess t, char** cs, size_t nCs, const char* mp, const char* m) {
	if (!t) {
		return false;
	}
	t->configPaths.reserve(nCs);
	for (auto i {0}; i < nCs; ++i) {
		t->configPaths.emplace_back(cs[i]);
	}
	t->modelPath = std::string(mp);
	t->model = std::string(m);

	return t->init();
}

Tess Tess_New() {
	return new ocr::Tesseract {};
}

void Tess_SetImage(Tess t, Proc p, int bytesPerPixel) {
	t->setImage(*p, bytesPerPixel);
}
