#include <cstring>
#include <vector>
#include <string>

#include "imgproc.h"

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

const unsigned char* Proc_EncodeImage(Proc p, const char* ext, int* encSize) {
	if (!p) {
		return nullptr;
	}
	const auto& enc {p->encodeImage(ext)};
	*encSize = static_cast<int>(enc.size());	// yolo
	return enc.data();
}

Img Proc_GetRawImage(Proc p) {
	if (!p) {
		return Img {};
	}
	return p->getRawImage().moveToC();
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
		beholder::Processor::OpList& list,
		void** ptrs,
		std::size_t nPtrs
	) -> void
	{
		list.reserve(nPtrs);
		for (auto i {0ul}; i < nPtrs; ++i) {
			list.emplace_back(static_cast<beholder::ProcessingOp*>(ptrs[i]));
		}
	};
	helper(p->postprocessing, post, nPost);
	helper(p->preprocessing, pre, nPre);
	return true;
}

Proc Proc_New() {
	return new beholder::Processor {};
}

bool Proc_Postprocess(Proc p, Res* res, size_t nRes) {
	if (!p || !res) {
		return false;
	}
	std::vector<beholder::Result> results;
	results.reserve(nRes);
	for (auto i {0ul}; i < nRes; ++i) {
		results.emplace_back(res[i]);
	}
	return p->postprocess(results);
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

bool Proc_ReceiveRawImage(Proc p, const Img* img) {
	if (!p || !img) {
		return false;
	}
	return p->receiveRawImage(beholder::RawImage {*img});
}

void Proc_ResetROI(Proc p) {
	if (!p) {
		return;
	}
	p->resetROI();
}

void Proc_SetROI(Proc p, const Rect* roi) {
	if (!p) {
		return;
	}
	p->setROI(*roi);
}

void Proc_SetRotatedROI(Proc p, const Rect* roi, double ang) {
	if (!p) {
		return;
	}
	p->setRotatedROI(*roi, ang);
}

void Proc_ShowImage(Proc p, const char* title) {
	if (!p) {
		return;
	}
	std::string s {title};
	p->showImage(s);
}

void Proc_ToColor(Proc p) {
	if (!p) {
		return;
	}
	p->toColor();
}

void Proc_ToGrayscale(Proc p) {
	if (!p) {
		return;
	}
	p->toGrayscale();
}

bool Proc_WriteImage(Proc p, const char* filename) {
	if (!p) {
		return false;
	}
	std::string s {filename};
	return p->writeImage(s);
}
