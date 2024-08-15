#pragma once

#include <string>

namespace env {
	void log(const std::u8string_view& str);
	void fail(const std::u8string_view& str);
}
