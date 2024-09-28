#include "interface.h"

#include <ustring/ustring.h>

void _interface::u8log(const std::u8string_view& str) {
	str::PrintLn(str);
}
void _interface::u8fail(const std::u8string_view& str) {
	str::FmtLn(u8"Exception: {}", str);
	exit(1);
}
