#include "logging.h"

void util::log(const std::u8string_view& msg) {
	str::PrintLn(str);
}

void util::fail(const std::u8string_view& str) {
	str::FmtLn(u8"Exception: {}", str);
	exit(1);
}
