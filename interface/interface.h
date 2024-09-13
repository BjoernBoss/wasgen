#pragma once

#include <string>

namespace env {
	void log(const std::u8string_view& str);
	void fail [[noreturn]] (const std::u8string_view& str);
}
