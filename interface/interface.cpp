#include "interface.h"

#include <ustring/str.h>

void env::log(const std::u8string_view& str) {
	str::OutLn(str);
}
void env::fail(const std::u8string_view& str) {
	str::FmtLn(u8"Exception: {}", str);
	exit(1);
}
