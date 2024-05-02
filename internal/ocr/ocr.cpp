#include <vector>
#include <string>

#include "ocr.h"


Cfg Cfg_New(const char* json) {
	Cfg c {new ocr::Config {}};
	if (!c->parse(json)) {
		delete c;
		c = nullptr;
	}
	return c;
}

Proc Proc_New() {
	return new ocr::ImageProcessor {};
}

bool Proc_Preprocess(Proc p, Cfg cfg) {
	if (!p || !cfg) {
		return false;
	}
	return p->preprocess(*cfg);
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

bool Tess_DetectText(Tess t, Proc p, Cfg cfg) {
	if (!t || !p || !cfg) {
		return false;
	}
	auto rects {t->detectText()};
	if (rects.empty()) {
		return false;
	}
	p->drawRectangles(rects, *cfg);

	return true;
}

Tess Tess_New(Cfg cfg) {
	if (!cfg) {
		return nullptr;
	}
	Tess t {new ocr::Tesseract {}};
	if (!t->init(*cfg)) {
		delete t;
		t = nullptr;
	}
	return t;
}

char* Tess_RecognizeText(Tess t) {
	if (!t) {
		return nullptr;
	}
	return t->recognizeText();
}

void Tess_SetImage(Tess t, Proc p, int bytesPerPixel) {
	t->setImage(*p, bytesPerPixel);
}
