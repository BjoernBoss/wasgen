#pragma once

#include <ustring/ustring.h>

#include "../interface/interface.h"

namespace util {
	/* log the single message */
	void log(const std::u8string_view& msg);

	/* build the message and log it */
	template <class... Args>
	void log(const Args&... args) {
		_env::u8log(str::Build<std::u8string>(args...));
	}

	/* build the message and log it and abort the entire execution */
	template <class... Args>
	void fail [[noreturn]] (const Args&... args) {
		_env::u8fail(str::Build<std::u8string>(args...));
	}
}
