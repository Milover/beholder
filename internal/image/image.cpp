#include <cstring>
#include <vector>
#include <string>

#include "image.h"


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

RawImage Proc_GetRawImage(Proc p) {
	if (!p) {
		return RawImage {};
	}
	beholder::RawImage img {p->getRawImage()};
	return RawImage {
		img.id,
		img.rows,
		img.cols,
		img.pixelType,
		img.buffer,
		img.step
	};
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

bool Proc_ReceiveRawImage(Proc p, const RawImage* img) {
	if (!p || !img) {
		return false;
	}
	beholder::RawImage ri {
		img->id,
		img->rows,
		img->cols,
		img->pxTyp,
		img->buf,
		img->step
	};
	return p->receiveRawImage(ri);
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
