// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

#include <algorithm>
#include <cctype>
#include <cstring>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

namespace beholder {

void chPtrFromLiteral(char*& ch, const char* lit) {
	delete[] (ch);

	auto len{std::strlen(lit)};
	ch = new char[len + 1];
	std::strncpy(ch, lit, len);
}

// NOLINTBEGIN(*-c-arrays)
std::unique_ptr<char*[]> vecStr2ChPtrArr(const std::vector<std::string>& v) {
	std::unique_ptr<char*[]> result{new char*[v.size() + 1]};

	for (auto i{0UL}; i < v.size(); ++i) {
		auto len{v[i].length()};
		result[i] = new char[len + 1];
		std::strncpy(result[i], v[i].c_str(), len);
	}
	result[v.size()] = nullptr;

	return result;
}
// NOLINTEND(*-c-arrays)

void trimWhiteL(std::string& s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
				return !static_cast<bool>(std::isspace(ch));
			}));
}

void trimWhiteR(std::string& s) {
	s.erase(std::find_if(
				s.rbegin(), s.rend(),
				[](int ch) { return !static_cast<bool>(std::isspace(ch)); })
				.base(),
			s.end());
}

void trimWhiteLR(std::string& s) {
	trimWhiteL(s);
	trimWhiteR(s);
}

}  // namespace beholder
