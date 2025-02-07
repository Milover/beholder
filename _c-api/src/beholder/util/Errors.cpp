// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#include "beholder/util/Errors.h"

#include <iostream>
#include <system_error>

namespace beholder {
namespace err {

std::error_code fromErrno(int errnoCode) {
	return std::make_error_code(static_cast<std::errc>(errnoCode));
};

void printErr(const std::error_code& err) {
	std::cerr << err.category().name() << " error (" << err.value()
			  << "): " << err.message() << std::endl;
}

}  // namespace err
}  // namespace beholder
