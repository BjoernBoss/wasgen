/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2024-2025 Bjoern Boss Henrichsen */
#include "binary-base.h"

uint32_t wasm::binary::CountUInt(uint64_t value) {
	uint32_t count = 1;
	while ((value >>= 7) != 0)
		++count;
	return count;
}
void wasm::binary::WriteInt32(std::vector<uint8_t>& buffer, uint32_t value) {
	binary::WriteSInt(buffer, int32_t(value));
}
void wasm::binary::WriteInt64(std::vector<uint8_t>& buffer, uint64_t value) {
	binary::WriteSInt(buffer, int64_t(value));
}
void wasm::binary::WriteUInt(std::vector<uint8_t>& buffer, uint64_t value) {
	do {
		uint8_t byte = uint8_t(value & 0x7f);
		if ((value >>= 7) != 0)
			byte |= 0x80;
		buffer.push_back(byte);
	} while (value != 0);
}
void wasm::binary::WriteSInt(std::vector<uint8_t>& buffer, int64_t value) {
	bool negative = (value < 0);
	uint64_t lastValue = (negative ? uint64_t(-1) : 0);
	uint8_t upperBit = (negative ? 0x40 : 0x00);

	while (true) {
		uint8_t byte = (value & 0x7f);
		if ((value >>= 7) != lastValue || (byte & 0x40) != upperBit)
			buffer.push_back(byte | 0x80);
		else {
			buffer.push_back(byte);
			break;
		}
	}
}
void wasm::binary::WriteFloat(std::vector<uint8_t>& buffer, float value) {
	const uint8_t* data = reinterpret_cast<const uint8_t*>(&value);
	buffer.insert(buffer.end(), data, data + sizeof(float));
}
void wasm::binary::WriteDouble(std::vector<uint8_t>& buffer, double value) {
	const uint8_t* data = reinterpret_cast<const uint8_t*>(&value);
	buffer.insert(buffer.end(), data, data + sizeof(double));

}
void wasm::binary::WriteBytes(std::vector<uint8_t>& buffer, std::initializer_list<uint8_t> bytes) {
	buffer.insert(buffer.end(), bytes.begin(), bytes.end());
}

uint8_t wasm::binary::GetType(wasm::Type type) {
	switch (type) {
	case wasm::Type::i32:
		return 0x7f;
	case wasm::Type::i64:
		return 0x7e;
	case wasm::Type::f32:
		return 0x7d;
	case wasm::Type::f64:
		return 0x7c;
	case wasm::Type::refFunction:
		return 0x70;
	case wasm::Type::refExtern:
		return 0x6f;
	default:
		throw wasm::Exception{ L"Unknown wasm type [", size_t(type), L"] encountered" };
	}
}
void wasm::binary::WriteString(std::vector<uint8_t>& buffer, std::u8string_view str) {
	binary::WriteUInt(buffer, str.size());
	const uint8_t* data = reinterpret_cast<const uint8_t*>(str.data());
	buffer.insert(buffer.end(), data, data + str.size());
}
void wasm::binary::WriteLimit(std::vector<uint8_t>& buffer, const wasm::Limit& limit) {
	buffer.push_back(limit.maxValid() ? 0x01 : 0x00);
	binary::WriteUInt(buffer, limit.min);
	if (limit.maxValid())
		binary::WriteUInt(buffer, limit.max);
}
void wasm::binary::WriteValue(std::vector<uint8_t>& buffer, const wasm::Value& value) {
	/* write the general instruction */
	switch (value.type()) {
	case wasm::ValType::i32:
		buffer.push_back(0x41);
		binary::WriteInt32(buffer, value.i32());
		break;
	case wasm::ValType::i64:
		buffer.push_back(0x42);
		binary::WriteInt64(buffer, value.i64());
		break;
	case wasm::ValType::f32:
		buffer.push_back(0x43);
		binary::WriteFloat(buffer, value.f32());
		break;
	case wasm::ValType::f64:
		buffer.push_back(0x44);
		binary::WriteDouble(buffer, value.f64());
		break;
	case wasm::ValType::refFunction:
		if (value.function().valid()) {
			buffer.push_back(0xd2);
			binary::WriteUInt(buffer, value.function().index());
		}
		else
			binary::WriteBytes(buffer, { 0xd0, binary::GetType(wasm::Type::refFunction) });
		break;
	case wasm::ValType::refExtern:
		binary::WriteBytes(buffer, { 0xd0, binary::GetType(wasm::Type::refExtern) });
		break;
	case wasm::ValType::global:
		buffer.push_back(0x23);
		binary::WriteUInt(buffer, value.global().index());
		break;
	default:
		throw wasm::Exception{ L"Unknown wasm val-type [", size_t(value.type()), L"] encountered" };
	}

	/* write the closing tag */
	buffer.push_back(0x0b);
}
