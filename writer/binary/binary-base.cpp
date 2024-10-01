#include "binary-base.h"

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
	case wasm::Type::v128:
		return 0x7b;
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
	binary::Write<uint32_t>(buffer, { uint32_t(str.size()) });
	const uint8_t* data = reinterpret_cast<const uint8_t*>(str.data());
	buffer.insert(buffer.end(), data, data + str.size());
}
void writer::binary::WriteLimit(std::vector<uint8_t>& buffer, const wasm::Limit& limit) {
	binary::Write<uint8_t>(buffer, { uint8_t(limit.maxValid() ? 0x01 : 0x00) });
	binary::Write<uint32_t>(buffer, { limit.min });
	if (limit.maxValid())
		binary::Write<uint32_t>(buffer, { limit.max });
}
