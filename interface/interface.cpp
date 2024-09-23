#include "interface.h"

#include <ustring/ustring.h>

void _env::u8log(const std::u8string_view& str) {
	str::PrintLn(str);
}
void _env::u8fail(const std::u8string_view& str) {
	str::FmtLn(u8"Exception: {}", str);
	exit(1);
}
