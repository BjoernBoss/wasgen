/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2024-2025 Bjoern Boss Henrichsen */
#pragma once

#include "../wasm-common.h"
#include "wasm-function.h"
#include "wasm-global.h"

namespace wasm {
	enum class ValType : uint8_t {
		invalid,
		i32,
		i64,
		f32,
		f64,
		refExtern,
		refFunction,
		global
	};

	class Value {
	private:
		std::variant<uint32_t, uint64_t, float, double, wasm::Function, wasm::Global> pValue;
		wasm::ValType pType = wasm::ValType::invalid;

	public:
		constexpr Value() : pValue{ uint32_t(0) }, pType{ wasm::ValType::invalid } {}

	private:
		constexpr Value(uint32_t v, wasm::ValType type) : pValue{ v }, pType{ type } {}
		constexpr Value(uint64_t v, wasm::ValType type) : pValue{ v }, pType{ type } {}
		constexpr Value(float v, wasm::ValType type) : pValue{ v }, pType{ type } {}
		constexpr Value(double v, wasm::ValType type) : pValue{ v }, pType{ type } {}
		constexpr Value(const wasm::Function& v, wasm::ValType type) : pValue{ v }, pType{ type } {}
		constexpr Value(const wasm::Global& v, wasm::ValType type) : pValue{ v }, pType{ type } {}

	public:
		static constexpr wasm::Value MakeExtern() {
			return wasm::Value{ uint32_t(0), wasm::ValType::refExtern };
		}
		static constexpr wasm::Value MakeFunction(const wasm::Function& function = {}) {
			return wasm::Value{ function, wasm::ValType::refFunction };
		}
		static constexpr wasm::Value MakeImported(const wasm::Global& global) {
			return wasm::Value{ global, wasm::ValType::global };
		}
		template <class Type>
		static constexpr wasm::Value MakeI32(Type value) {
			return wasm::Value{ uint32_t(int32_t(value)), wasm::ValType::i32 };
		}
		template <class Type>
		static constexpr wasm::Value MakeU32(Type value) {
			return wasm::Value{ uint32_t(value), wasm::ValType::i32 };
		}
		template <class Type>
		static constexpr wasm::Value MakeI64(Type value) {
			return wasm::Value{ uint64_t(int64_t(value)), wasm::ValType::i64 };
		}
		template <class Type>
		static constexpr wasm::Value MakeU64(Type value) {
			return wasm::Value{ uint64_t(value), wasm::ValType::i64 };
		}
		template <class Type>
		static constexpr wasm::Value MakeF32(Type value) {
			return wasm::Value{ float(value), wasm::ValType::f32 };
		}
		template <class Type>
		static constexpr wasm::Value MakeF64(Type value) {
			return wasm::Value{ double(value), wasm::ValType::f64 };
		}

	public:
		bool valid() const {
			return (pType != wasm::ValType::invalid);
		}
		wasm::ValType type() const {
			return pType;
		}
		uint32_t i32() const {
			return std::get<uint32_t>(pValue);
		}
		uint64_t i64() const {
			return std::get<uint64_t>(pValue);
		}
		float f32() const {
			return std::get<float>(pValue);
		}
		double f64() const {
			return std::get<double>(pValue);
		}
		wasm::Function function() const {
			return std::get<wasm::Function>(pValue);
		}
		wasm::Global global() const {
			return std::get<wasm::Global>(pValue);
		}
	};
}
