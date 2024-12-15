/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2024 Bjoern Boss Henrichsen */
#pragma once

#include <ustring/ustring.h>
#include <algorithm>
#include <vector>

#include "../../objects/wasm-module.h"
#include "../../sink/wasm-sink.h"
#include "../../inst/wasm-instlist.h"

namespace wasm::binary {
	class Module;
	class Sink;

	uint32_t CountUInt(uint64_t value);
	void WriteInt32(std::vector<uint8_t>& buffer, uint32_t value);
	void WriteInt64(std::vector<uint8_t>& buffer, uint64_t value);
	void WriteUInt(std::vector<uint8_t>& buffer, uint64_t value);
	void WriteSInt(std::vector<uint8_t>& buffer, int64_t value);
	void WriteFloat(std::vector<uint8_t>& buffer, float value);
	void WriteDouble(std::vector<uint8_t>& buffer, double value);
	void WriteBytes(std::vector<uint8_t>& buffer, std::initializer_list<uint8_t> bytes);

	uint8_t GetType(wasm::Type type);
	void WriteString(std::vector<uint8_t>& buffer, std::u8string_view str);
	void WriteLimit(std::vector<uint8_t>& buffer, const wasm::Limit& limit);
	void WriteValue(std::vector<uint8_t>& buffer, const wasm::Value& value);
}
