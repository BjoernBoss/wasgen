#include "binary-base.h"

uint32_t writer::binary::CountUInt(uint64_t value) {
	uint32_t count = 1;
	while (true) {
		value >>= 7;
		if (value == 0)
			return count;
		++count;
	}
}
void writer::binary::WriteUInt(std::vector<uint8_t>& buffer, uint64_t value) {
	while (true) {
		bool last = (value < 0x80);
		buffer.push_back(uint8_t(value & 0x7f) | (last ? 0x00 : 0x80));
		value >>= 7;
		if (last)
			break;
	}
}
void writer::binary::WriteSInt(std::vector<uint8_t>& buffer, int64_t value) {
	uint64_t walker = (value < 0 ? uint64_t(~value) + 1 : uint64_t(value));

	while (true) {
		bool last = ((walker >>= 7) == 0);
		buffer.push_back(uint8_t(value & 0x7f) | (last ? 0x00 : 0x80));
		value >>= 7;
		if (last)
			break;
	}
}
void writer::binary::WriteFloat(std::vector<uint8_t>& buffer, float value) {
	const uint8_t* data = reinterpret_cast<const uint8_t*>(&value);
	buffer.insert(buffer.end(), data, data + sizeof(float));
}
void writer::binary::WriteDouble(std::vector<uint8_t>& buffer, double value) {
	const uint8_t* data = reinterpret_cast<const uint8_t*>(&value);
	buffer.insert(buffer.end(), data, data + sizeof(double));

}
void writer::binary::WriteBytes(std::vector<uint8_t>& buffer, std::initializer_list<uint8_t> bytes) {
	buffer.insert(buffer.end(), bytes.begin(), bytes.end());
}

uint8_t writer::binary::GetType(wasm::Type type) {
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
		util::fail(u8"Unknown wasm type [", size_t(type), u8"] encountered");
		return 0;
	}
}
void writer::binary::WriteString(std::vector<uint8_t>& buffer, std::u8string_view str) {
	binary::WriteUInt(buffer, str.size());
	const uint8_t* data = reinterpret_cast<const uint8_t*>(str.data());
	buffer.insert(buffer.end(), data, data + str.size());
}
void writer::binary::WriteLimit(std::vector<uint8_t>& buffer, const wasm::Limit& limit) {
	buffer.push_back(limit.maxValid() ? 0x01 : 0x00);
	binary::WriteUInt(buffer, limit.min);
	if (limit.maxValid())
		binary::WriteUInt(buffer, limit.max);
}
void writer::binary::WriteValue(std::vector<uint8_t>& buffer, const wasm::Value& value) {
	/* write the general instruction */
	switch (value.type()) {
	case wasm::ValType::i32:
		buffer.push_back(0x41);
		binary::WriteUInt(buffer, value.i32());
		break;
	case wasm::ValType::i64:
		buffer.push_back(0x42);
		binary::WriteUInt(buffer, value.i64());
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
		util::fail(u8"Unknown wasm val-type [", size_t(value.type()), u8"] encountered");
		break;
	}

	/* write the closing tag */
	buffer.push_back(0x0b);
}
