#include <cstring>
#include <vector>
#include <string>

#include "ocr.h"


bool Proc_CopyBayerRGGB8(Proc p, int rows, int cols, void* buf, size_t step) {
	if (!p) {
		return false;
	}
	try {
		p->copyBayerRGGB8(rows, cols, buf, step);
		return true;
	} catch(...) {}
	return false;
}

bool Proc_DecodeImage(Proc p, void* buf, int bufSize, int flags) {
	if (!p) {
		return false;
	}
	if (!buf && bufSize > 0) {
		return false;
	}
	return p->decodeImage(buf, bufSize, flags);
}

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
	if ((!post && nPost > 0) || (!pre && nPre > 0)) {
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

bool Proc_ReceiveMono8(Proc p, int rows, int cols, void* buf, size_t step) {
	if (!p) {
		return false;
	}
	try {
		p->receiveMono8(rows, cols, buf, step);
		return true;
	} catch(...) {}
	return false;
}

void Proc_ShowImage(Proc p, const char* title) {
	if (!p) {
		return;
	}
	std::string s {title};
	p->showImage(s);
}

bool Proc_WriteImage(Proc p, const char* filename) {
	if (!p) {
		return false;
	}
	std::string s {filename};
	return p->writeImage(s);
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

bool Tess_Init(Tess t, const TInit* in) {
	if (!t || !in) {
		return false;
	}
	if (!in->cfgs && in->nCfgs > 0) {
		return false;
	}
	t->modelPath = std::string(in->modelPath);
	t->model = std::string(in->model);
	t->pageSegMode = in->psMode;
	// handle configs
	t->configPaths.clear();
	t->configPaths.reserve(in->nCfgs);
	for (auto i {0ul}; i < in->nCfgs; ++i) {
		t->configPaths.emplace_back(in->cfgs[i]);
	}
	// handle variables
	t->variables.clear();
	t->variables.reserve(in->nVars);
	for (auto i {0ul}; i < in->nVars; ++i) {
		t->variables.emplace_back(in->vars[i].key, in->vars[i].value);
	}
	return t->init();
}

Tess Tess_New() {
	return new ocr::Tesseract {};
}

void Tess_SetImage(Tess t, Proc p, int bytesPerPixel) {
	if (!t || !p) {
		return;
	}
	t->setImage(*p, bytesPerPixel);
}
