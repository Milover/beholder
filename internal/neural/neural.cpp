#include <array>
#include <cstring>
#include <vector>
#include <string>

#include "neural.h"

// vecAsgn assigns values from C-array to a std::array.
template<typename T, int Sz>
void arAsgn(std::array<T, Sz>& ar, const T (&a)[Sz]) {
	for (auto i {0}; i < Sz; ++i) {
		ar[i] = a[i];
	}
}

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
	try {
		d->modelPath = std::string {in->modelPath};
		d->model = std::string {in->model};
		d->backend = in->backend;
		d->target = in->target;
		d->confidenceThreshold = in->conf;
		d->nmsThreshold = in->nms;
		d->swapRB = in->swapRB;
		// handle arrays
		arAsgn<double, 3>(d->mean, in->mean);
		arAsgn<double, 3>(d->padValue, in->pad);
		arAsgn<double, 3>(d->scale, in->scale);
		arAsgn<int, 2>(d->size, in->size);

		return d->init();
	} catch(...) {
		// cleanup ?
	}
	return false;
}

Det Det_NewCRAFT() {
	return static_cast<Det>(new beholder::CRAFTDetector {});
}

Det Det_NewEAST() {
	return static_cast<Det>(new beholder::EASTDetector {});
}

Det Det_NewPARSeq() {
	return static_cast<Det>(new beholder::PARSeqDetector {});
}

Det Det_NewYOLOv8() {
	return static_cast<Det>(new beholder::YOLOv8Detector {});
}

bool Det_ConfigureCRAFT(Det d, float txtThresh, float lnThresh, float lowTxt) {
	using CRAFT = beholder::CRAFTDetector;
	CRAFT* ptr {dynamic_cast<CRAFT*>(d)};	// futureproofing
	if (!ptr) {
		return false;
	}
	// handle threshold values
	ptr->textThreshold = txtThresh;
	ptr->linkThreshold = lnThresh;
	ptr->lowText = lowTxt;
	return true;
}

bool Det_ConfigurePARSeq(Det d, const char* charset) {
	using PARSeq = beholder::PARSeqDetector;
	PARSeq* ptr {dynamic_cast<PARSeq*>(d)};
	if (!ptr) {
		return false;
	}
	if (!charset) {
		return false;
	}
	// handle the charset
	ptr->charset = charset;
	return true;
}

bool Det_ConfigureYOLOv8(Det d, const char** classes, size_t nClasses) {
	using YOLOv8 = beholder::YOLOv8Detector;
	YOLOv8* ptr {dynamic_cast<YOLOv8*>(d)};	// futureproofing
	if (!ptr) {
		return false;
	}
	if (!classes || nClasses == 0) {	// valid; nothing to do
		return true;
	}
	// handle classes
	ptr->classes.clear();
	ptr->classes.reserve(nClasses);
	for (auto i {0ul}; i < nClasses; ++i) {
		ptr->classes.emplace_back(classes[i]);
	}
	return true;
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
