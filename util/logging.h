#pragma once

#include <ustring/ustring.h>

namespace util {
	/* log the single message */
	void log(const std::u8string_view& msg);

	/* fail with single message */
	void fail(const std::u8string_view& msg);

	/* build the message and log it */
	template <class... Args>
	void log(const Args&... args) {
		util::log(str::Build<std::u8string>(args...));
	}

	/* build the message and log it and abort the entire execution */
	template <class... Args>
	void fail [[noreturn]] (const Args&... args) {
		util::log(str::Build<std::u8string>(args...));
	}
}
