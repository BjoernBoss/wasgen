#include "interface.h"

#include <ustring/ustring.h>

void env::u8log(const std::u8string_view& str) {
	str::PrintLn(str);
}
void env::u8fail(const std::u8string_view& str) {
	str::FmtLn(u8"Exception: {}", str);
	exit(1);
}
