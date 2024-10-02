#pragma once

#include <ustring/ustring.h>
#include <algorithm>
#include <vector>

#include "../wasm/wasm.h"
#include "../util/logging.h"

namespace writer::binary {
	class Module;
	class Sink;

	uint32_t CountUInt(uint64_t value);
	void WriteUInt(std::vector<uint8_t>& buffer, uint64_t value);
	void WriteSInt(std::vector<uint8_t>& buffer, int64_t value);
	void WriteFloat(std::vector<uint8_t>& buffer, float value);
	void WriteDouble(std::vector<uint8_t>& buffer, double value);
	void WriteBytes(std::vector<uint8_t>& buffer, std::initializer_list<uint8_t> bytes);

	uint8_t GetType(wasm::Type type);
	void WriteString(std::vector<uint8_t>& buffer, std::u8string_view str);
	void WriteLimit(std::vector<uint8_t>& buffer, const wasm::Limit& limit);
}
