#pragma once

#include "wasm-instruction.h"

namespace wasm::detail {
	template <wasm::OpType Type, bool Signed>
	struct Constant {
		template <class ValType>
		static constexpr wasm::InstConst Const(ValType val) {
			if constexpr (Type == wasm::OpType::i32) {
				if constexpr (Signed)
					return wasm::InstConst{ uint32_t(int32_t(val)), wasm::OpType::i32 };
				return wasm::InstConst{ uint32_t(val), wasm::OpType::i32 };
			}
			else if constexpr (Type == wasm::OpType::i64) {
				if constexpr (Signed)
					return wasm::InstConst{ uint64_t(int64_t(val)), wasm::OpType::i64 };
				return wasm::InstConst{ uint64_t(val), wasm::OpType::i64 };
			}
			else if constexpr (Type == wasm::OpType::f32)
				return wasm::InstConst{ float(val), wasm::OpType::f32 };
			else
				return wasm::InstConst{ double(val), wasm::OpType::f64 };
		}
	};

	template < wasm::OpType Type, bool Signed>
	struct Compare {
		static constexpr wasm::InstSimple Equal() {
			return wasm::InstSimple{ wasm::InstSimple::Type::equal, Type };
		}
		static constexpr wasm::InstSimple EqualZero() {
			return wasm::InstSimple{ wasm::InstSimple::Type::equalZero, Type };
		}
		static constexpr wasm::InstSimple NotEqual() {
			return wasm::InstSimple{ wasm::InstSimple::Type::notEqual, Type };
		}
		static constexpr wasm::InstSimple Greater() {
			if constexpr (Signed)
				return wasm::InstSimple{ wasm::InstSimple::Type::greaterSigned, Type };
			else
				return wasm::InstSimple{ wasm::InstSimple::Type::greaterUnsigned, Type };
		}
		static constexpr wasm::InstSimple Less() {
			if constexpr (Signed)
				return wasm::InstSimple{ wasm::InstSimple::Type::lessSigned, Type };
			else
				return wasm::InstSimple{ wasm::InstSimple::Type::lessUnsigned, Type };
		}
		static constexpr wasm::InstSimple GreaterEqual() {
			if constexpr (Signed)
				return wasm::InstSimple{ wasm::InstSimple::Type::greaterEqualSigned, Type };
			else
				return wasm::InstSimple{ wasm::InstSimple::Type::greaterEqualUnsigned, Type };
		}
		static constexpr wasm::InstSimple LessEqual() {
			if constexpr (Signed)
				return wasm::InstSimple{ wasm::InstSimple::Type::lessEqualSigned, Type };
			else
				return wasm::InstSimple{ wasm::InstSimple::Type::lessEqualUnsigned, Type };
		}
	};

	template <wasm::OpType Type, bool Signed>
	struct Arithmetic {
		static constexpr wasm::InstSimple Add() {
			return wasm::InstSimple{ wasm::InstSimple::Type::add, Type };
		}
		static constexpr wasm::InstSimple Sub() {
			return wasm::InstSimple{ wasm::InstSimple::Type::sub, Type };
		}
		static constexpr wasm::InstSimple Mul() {
			return wasm::InstSimple{ wasm::InstSimple::Type::mul, Type };
		}
		static constexpr wasm::InstSimple Div() {
			if constexpr (Signed)
				return wasm::InstSimple{ wasm::InstSimple::Type::divSigned, Type };
			else
				return wasm::InstSimple{ wasm::InstSimple::Type::divUnsigned, Type };
		}
		static constexpr wasm::InstSimple Mod() {
			if constexpr (Signed)
				return wasm::InstSimple{ wasm::InstSimple::Type::modSigned, Type };
			else
				return wasm::InstSimple{ wasm::InstSimple::Type::modUnsigned, Type };
		}
	};

	template <wasm::OpType Type>
	struct SmallConvert {
		static constexpr wasm::InstSimple Expand() {
			return wasm::InstSimple{ wasm::InstSimple::Type::expand, Type };
		}
	};

	template <wasm::OpType Type>
	struct LargeConvert {
		static constexpr wasm::InstSimple Shrink() {
			return wasm::InstSimple{ wasm::InstSimple::Type::shrink, Type };
		}
	};

	template <wasm::OpType Type>
	struct FloatConvert {
		static constexpr wasm::InstSimple AsInt() {
			return wasm::InstSimple{ wasm::InstSimple::Type::reinterpretAsInt, Type };
		}
	};

	template <wasm::OpType Type, bool Signed>
	struct IntConvert {
		static constexpr wasm::InstSimple ToF32() {
			if constexpr (Signed)
				return wasm::InstSimple{ wasm::InstSimple::Type::convertToF32Signed, Type };
			else
				return wasm::InstSimple{ wasm::InstSimple::Type::convertToF32Unsigned, Type };
		}
		static constexpr wasm::InstSimple ToF64() {
			if constexpr (Signed)
				return wasm::InstSimple{ wasm::InstSimple::Type::convertToF64Signed, Type };
			else
				return wasm::InstSimple{ wasm::InstSimple::Type::convertToF64Unsigned, Type };
		}
		static constexpr wasm::InstSimple FromF32() {
			if constexpr (Signed)
				return wasm::InstSimple{ wasm::InstSimple::Type::convertFromF32Signed, Type };
			else
				return wasm::InstSimple{ wasm::InstSimple::Type::convertFromF32Unsigned, Type };
		}
		static constexpr wasm::InstSimple FromF64() {
			if constexpr (Signed)
				return wasm::InstSimple{ wasm::InstSimple::Type::convertFromF64Signed, Type };
			else
				return wasm::InstSimple{ wasm::InstSimple::Type::convertFromF64Unsigned, Type };
		}
		static constexpr wasm::InstSimple AsFloat() {
			return wasm::InstSimple{ wasm::InstSimple::Type::reinterpretAsFloat, Type };
		}
	};

	template <wasm::OpType Type, bool Signed>
	struct Bitwise {
		static constexpr wasm::InstSimple And() {
			return wasm::InstSimple{ wasm::InstSimple::Type::bitAnd, Type };
		}
		static constexpr wasm::InstSimple Or() {
			return wasm::InstSimple{ wasm::InstSimple::Type::bitOr, Type };
		}
		static constexpr wasm::InstSimple XOr() {
			return wasm::InstSimple{ wasm::InstSimple::Type::bitXOr, Type };
		}
		static constexpr wasm::InstSimple ShiftLeft() {
			return wasm::InstSimple{ wasm::InstSimple::Type::bitShiftLeft, Type };
		}
		static constexpr wasm::InstSimple ShiftRight() {
			if constexpr (Signed)
				return wasm::InstSimple{ wasm::InstSimple::Type::bitShiftRightSigned, Type };
			else
				return wasm::InstSimple{ wasm::InstSimple::Type::bitShiftRightUnsigned, Type };
		}
		static constexpr wasm::InstSimple RotateLeft() {
			return wasm::InstSimple{ wasm::InstSimple::Type::bitRotateLeft, Type };
		}
		static constexpr wasm::InstSimple RotateRight() {
			return wasm::InstSimple{ wasm::InstSimple::Type::bitRotateRight, Type };
		}
		static constexpr wasm::InstSimple LeadingNulls() {
			return wasm::InstSimple{ wasm::InstSimple::Type::bitLeadingNulls, Type };
		}
		static constexpr wasm::InstSimple TrailingNulls() {
			return wasm::InstSimple{ wasm::InstSimple::Type::bitTrailingNulls, Type };
		}
		static constexpr wasm::InstSimple SetBits() {
			return wasm::InstSimple{ wasm::InstSimple::Type::bitSetCount, Type };
		}
	};

	template <wasm::OpType Type>
	struct Float {
		static constexpr wasm::InstSimple Min() {
			return wasm::InstSimple{ wasm::InstSimple::Type::floatMin, Type };
		}
		static constexpr wasm::InstSimple Max() {
			return wasm::InstSimple{ wasm::InstSimple::Type::floatMax, Type };
		}
		static constexpr wasm::InstSimple Floor() {
			return wasm::InstSimple{ wasm::InstSimple::Type::floatFloor, Type };
		}
		static constexpr wasm::InstSimple Round() {
			return wasm::InstSimple{ wasm::InstSimple::Type::floatRound, Type };
		}
		static constexpr wasm::InstSimple Ceil() {
			return wasm::InstSimple{ wasm::InstSimple::Type::floatCeil, Type };
		}
		static constexpr wasm::InstSimple Truncate() {
			return wasm::InstSimple{ wasm::InstSimple::Type::floatTruncate, Type };
		}
		static constexpr wasm::InstSimple Absolute() {
			return wasm::InstSimple{ wasm::InstSimple::Type::floatAbsolute, Type };
		}
		static constexpr wasm::InstSimple Negate() {
			return wasm::InstSimple{ wasm::InstSimple::Type::floatNegate, Type };
		}
		static constexpr wasm::InstSimple SquareRoot() {
			return wasm::InstSimple{ wasm::InstSimple::Type::floatSquareRoot, Type };
		}
		static constexpr wasm::InstSimple CopySign() {
			return wasm::InstSimple{ wasm::InstSimple::Type::floatCopySign, Type };
		}
	};

	template <wasm::OpType Type>
	struct Memory {
		static constexpr wasm::InstMemory Load(uint32_t offset = 0) {
			return wasm::InstMemory{ wasm::InstMemory::Type::load, {}, {}, offset, Type };
		}
		static constexpr wasm::InstMemory Store(uint32_t offset = 0) {
			return wasm::InstMemory{ wasm::InstMemory::Type::store, {}, {}, offset, Type };
		}
		static constexpr wasm::InstMemory Load(const wasm::Memory& memory, uint32_t offset = 0) {
			return wasm::InstMemory{ wasm::InstMemory::Type::load, memory, {}, offset, Type };
		}
		static constexpr wasm::InstMemory Store(const wasm::Memory& memory, uint32_t offset = 0) {
			return wasm::InstMemory{ wasm::InstMemory::Type::store, memory, {}, offset, Type };
		}
	};

	template <wasm::OpType Type, bool Signed>
	struct LargeMemory {
		static constexpr wasm::InstMemory Load32(const wasm::Memory& memory = {}, uint32_t offset = 0) {
			if constexpr (Signed)
				return wasm::InstMemory{ wasm::InstMemory::Type::load32Signed, memory, {}, offset, Type };
			else
				return wasm::InstMemory{ wasm::InstMemory::Type::load32Unsigned, memory, {}, offset, Type };
		}
		static constexpr wasm::InstMemory Store32(const wasm::Memory& memory = {}, uint32_t offset = 0) {
			return wasm::InstMemory{ wasm::InstMemory::Type::store32, memory, {}, offset, Type };
		}
	};

	template <wasm::OpType Type, bool Signed>
	struct IntMemory {
		static constexpr wasm::InstMemory Load8(const wasm::Memory& memory = {}, uint32_t offset = 0) {
			if constexpr (Signed)
				return wasm::InstMemory{ wasm::InstMemory::Type::load8Signed, memory, {}, offset, Type };
			else
				return wasm::InstMemory{ wasm::InstMemory::Type::load8Unsigned, memory, {}, offset, Type };
		}
		static constexpr wasm::InstMemory Load16(const wasm::Memory& memory = {}, uint32_t offset = 0) {
			if constexpr (Signed)
				return wasm::InstMemory{ wasm::InstMemory::Type::load16Signed, memory, {}, offset, Type };
			else
				return wasm::InstMemory{ wasm::InstMemory::Type::load16Unsigned, memory, {}, offset, Type };
		}
		static constexpr wasm::InstMemory Store8(const wasm::Memory& memory = {}, uint32_t offset = 0) {
			return wasm::InstMemory{ wasm::InstMemory::Type::store8, memory, {}, offset, Type };
		}
		static constexpr wasm::InstMemory Store16(const wasm::Memory& memory = {}, uint32_t offset = 0) {
			return wasm::InstMemory{ wasm::InstMemory::Type::store16, memory, {}, offset, Type };
		}
	};
}
