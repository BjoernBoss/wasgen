/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2024 Bjoern Boss Henrichsen */
#include "text-base.h"

std::u8string_view writer::text::MakeType(wasm::Type type) {
	switch (type) {
	case wasm::Type::i32:
		return u8" i32";
	case wasm::Type::i64:
		return u8" i64";
	case wasm::Type::f32:
		return u8" f32";
	case wasm::Type::f64:
		return u8" f64";
	case wasm::Type::refExtern:
		return u8" externref";
	case wasm::Type::refFunction:
		return u8" funcref";
	default:
		throw wasm::Exception{ L"Unknown wasm type [", size_t(type), L"] encountered" };
	}
}
std::u8string writer::text::MakeId(std::u8string_view id) {
	if (id.empty())
		return {};
	return str::Build<std::u8string>(u8" $", id);
}
std::u8string writer::text::MakeExport(bool exported, std::u8string_view id) {
	if (!exported)
		return {};
	return str::Build<std::u8string>(u8" (export \"", id, u8"\")");
}
std::u8string writer::text::MakeImport(const std::u8string& importModule, std::u8string_view id) {
	if (importModule.empty())
		return {};
	return str::Build<std::u8string>(u8" (import \"", importModule, u8"\" \"", id, u8"\")");
}
std::u8string writer::text::MakeLimit(const wasm::Limit& limit) {
	if (limit.maxValid())
		return str::Build<std::u8string>(u8' ', limit.min, u8' ', limit.max);
	return str::Build<std::u8string>(u8' ', limit.min);
}
std::u8string writer::text::MakePrototype(const wasm::Prototype& prototype) {
	return str::Build<std::u8string>(u8" (type ", prototype.toString(), u8')');
}

std::u8string_view writer::text::MakeOperand(wasm::OpType operand) {
	switch (operand) {
	case wasm::OpType::i32:
		return u8"i32";
	case wasm::OpType::i64:
		return u8"i64";
	case wasm::OpType::f32:
		return u8"f32";
	case wasm::OpType::f64:
		return u8"f64";
	default:
		throw wasm::Exception{ L"Unknown operand type [", size_t(operand), L"] encountered" };
	}
}
std::u8string writer::text::MakeValue(const wasm::Value& value) {
	switch (value.type()) {
	case wasm::ValType::i32:
		return str::Build<std::u8string>(u8"i32.const ", value.i32());
	case wasm::ValType::i64:
		return str::Build<std::u8string>(u8"i64.const ", value.i64());
	case wasm::ValType::f32:
		return str::Build<std::u8string>(u8"f32.const ", value.f32());
	case wasm::ValType::f64:
		return str::Build<std::u8string>(u8"f64.const ", value.f64());
	case wasm::ValType::refExtern:
		return u8"ref.null extern";
	case wasm::ValType::refFunction:
		if (value.function().valid())
			return str::Build<std::u8string>(u8"ref.func ", value.function().toString());
		return u8"ref.null func";
	case wasm::ValType::global:
		return str::Build<std::u8string>(u8"global.get ", value.global().toString());
	case wasm::ValType::invalid:
	default:
		throw wasm::Exception{ L"Unknown value type [", size_t(value.type()), L"] encountered" };
	}
}
