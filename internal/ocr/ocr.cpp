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

void Det_Clear(Det d) {
	if (d) {
		d->clear();
	}
}

void Det_Delete(Det d) {
	if (d) {
		delete d;
		d = nullptr;
	}
}

ResArr* Det_Detect(Det d, const Img* img) {
	if (!d || !img) {
		return nullptr;
	}
	if (!d->detect(beholder::RawImage {*img})) {
		return nullptr;
	}
	const std::vector<beholder::Result>& results {d->getResults()};
	Res* res {new Res[results.size()]};
	for (auto i {0ul}; i < results.size(); ++i) {
		res[i] = results[i].toC();
	}
	return new ResArr{res, static_cast<size_t>(results.size())};
}

bool Det_Init(Det d, const DetInit* in) {
	if (!d || !in) {
		return false;
	}
	auto arrAsgn = [](std::array<double, 3>& ar, const double (&a)[3]) -> void {
		for (auto i {0}; i < 3; ++i) {
			ar[i] = a[i];
		}
	};
	d->modelPath = std::string {in->modelPath};
	d->model = std::string {in->model};
	d->backend = in->backend;
	d->target = in->target;
	d->size = in->size;
	d->scale = in->scale;
	d->confidenceThreshold = in->conf;
	d->nmsThreshold = in->nms;
	d->swapRB = in->swapRB;
	// handle arrays
	arrAsgn(d->mean, in->mean);
	arrAsgn(d->padValue, in->pad);
	// handle classes
	d->classes.clear();
	d->classes.reserve(in->nClasses);
	for (auto i {0ul}; i < in->nClasses; ++i) {
		d->classes.emplace_back(in->classes[i]);
	}
	return d->init();
}

Det Det_NewEAST() {
	return static_cast<Det>(new beholder::EASTDetector {});
}

Det Det_NewYOLOv8() {
	return static_cast<Det>(new beholder::YOLOv8Detector {});
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
		res[i] = results[i].toC();
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

bool Tess_SetImage(Tess t, const Img* img) {
	if (!t || !img) {
		return false;
	}
	return t->setImage(beholder::RawImage {*img});
}
