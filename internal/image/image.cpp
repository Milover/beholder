#include <cstring>
#include <vector>
#include <string>

#include <pylon/GrabResultPtr.h>

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

bool Proc_ReceiveAcquisitionResult(Proc p, const void* result) {
	if (!p || !result) {
		return false;
	}
	return p->receiveAcquisitionResult
	(
		*static_cast<const Pylon::CGrabResultPtr*>(result)
	);
}

void Proc_ShowImage(Proc p, const char* title) {
	if (!p) {
		return;
	}
	std::string s {title};
	p->showImage(s);
}

bool Proc_WriteAcquisitionResult(Proc p, const void* result, const char* filename) {
	if (!p || !result) {
		return false;
	}
	std::string s {filename};
	return p->writeAcquisitionResult(
		*static_cast<const Pylon::CGrabResultPtr*>(result),
		filename
	);
}

bool Proc_WriteImage(Proc p, const char* filename) {
	if (!p) {
		return false;
	}
	std::string s {filename};
	return p->writeImage(s);
}
