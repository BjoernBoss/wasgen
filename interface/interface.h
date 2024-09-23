#pragma once

#include <string>

namespace _env {
	void u8log(const std::u8string_view& str);
	void u8fail [[noreturn]] (const std::u8string_view& str);
}
