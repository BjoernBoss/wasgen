#pragma once

#include <ustring/ustring.h>
#include <algorithm>
#include <vector>

#include "../wasm/wasm.h"
#include "../util/logging.h"

namespace writer::binary {
	class Module;
	class Sink;

	template <class Type>
	void Write(std::vector<uint8_t>& buffer, std::initializer_list<Type> values) {
		const uint8_t* data = reinterpret_cast<const uint8_t*>(values.begin());
		buffer.insert(buffer.end(), data, data + values.size() * sizeof(Type));
	}
	uint8_t GetType(wasm::Type type);
	void WriteString(std::vector<uint8_t>& buffer, std::u8string_view str);
	void WriteLimit(std::vector<uint8_t>& buffer, const wasm::Limit& limit);
}
