#include "logging.h"

void util::log(const std::u8string_view& msg) {
	_interface::u8log(msg);
}
