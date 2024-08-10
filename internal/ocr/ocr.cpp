#include <cstring>
#include <vector>
#include <string>

#include "ocr.h"

void ResArr_Delete(void* r) {
	if (r) {
		ResArr** ptr {static_cast<ResArr**>(r)};
		if (ResArr* p {*ptr}; p) {
			if (p->array)
			// delete text of each result
			for (auto i {0ul}; i < p->count; ++i) {
				delete[] p->array[i].text;
			}
			// delete the underlying array
			delete[] p->array;
			p->array = nullptr;
		}
		// delete the wrapper
		delete *ptr;
		*ptr = nullptr;
	}
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

ResArr* Tess_Recognize(Tess t) {
	if (!t) {
		return nullptr;
	}
	if (!t->recognizeText()) {
		return nullptr;
	}
	const std::vector<beholder::Result>& results {t->getResults()};
	Res* res {new Res[results.size()]};
	for (auto i {0ul}; i < results.size(); ++i) {
		const beholder::Result& raw {results[i]};
		Res& ret {res[i]};

		// copy the string
		ret.text = new char[raw.text.size() + 1];
		std::strncpy(ret.text, raw.text.c_str(), raw.text.size() + 1);

		// copy the other stuff
		ret.conf = raw.confidence;
		ret.box.left = raw.box.left;
		ret.box.top = raw.box.top;
		ret.box.right = raw.box.right;
		ret.box.bottom = raw.box.bottom;
	}
	return new ResArr{res, static_cast<size_t>(results.size())};
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
	return new beholder::Tesseract {};
}

void Tess_SetImage(Tess t, Proc p, int bytesPerPixel) {
	if (!t || !p) {
		return;
	}
	t->setImage(*p, bytesPerPixel);
}
