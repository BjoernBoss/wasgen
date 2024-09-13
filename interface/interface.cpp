#include "interface.h"

#include <ustring/str.h>

void env::log(const std::u8string_view& str) {
	str::PrintLn(str);
}
void env::fail(const std::u8string_view& str) {
	str::FmtLn(u8"Exception: {}", str);
	exit(1);
}
