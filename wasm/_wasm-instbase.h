#pragma once

#include "_wasm-instruction.h"

namespace wasm::detail {
	template <wasm::_OpType Type, bool Signed>
	struct _Constant {
		template <class ValType>
		static constexpr wasm::_InstConst Const(ValType val) {
			if constexpr (Type == wasm::_OpType::i32) {
				if constexpr (Signed)
					return wasm::_InstConst{ uint32_t(int32_t(val)), wasm::_OpType::i32 };
				return wasm::_InstConst{ uint32_t(val), wasm::_OpType::i32 };
			}
			else if constexpr (Type == wasm::_OpType::i64) {
				if constexpr (Signed)
					return wasm::_InstConst{ uint64_t(int64_t(val)), wasm::_OpType::i64 };
				return wasm::_InstConst{ uint64_t(val), wasm::_OpType::i64 };
			}
			else if constexpr (Type == wasm::_OpType::f32)
				return wasm::_InstConst{ float(val), wasm::_OpType::f32 };
			else
				return wasm::_InstConst{ double(val), wasm::_OpType::f64 };
		}
	};

	template < wasm::_OpType Type, bool Signed>
	struct _Compare {
		static constexpr wasm::_InstSimple Equal() {
			return wasm::_InstSimple{ wasm::_InstSimple::Type::equal, Type };
		}
		static constexpr wasm::_InstSimple EqualZero() {
			return wasm::_InstSimple{ wasm::_InstSimple::Type::equalZero, Type };
		}
		static constexpr wasm::_InstSimple NotEqual() {
			return wasm::_InstSimple{ wasm::_InstSimple::Type::notEqual, Type };
		}
		static constexpr wasm::_InstSimple Greater() {
			if constexpr (Signed)
				return wasm::_InstSimple{ wasm::_InstSimple::Type::greaterSigned, Type };
			else
				return wasm::_InstSimple{ wasm::_InstSimple::Type::greaterUnsigned, Type };
		}
		static constexpr wasm::_InstSimple Less() {
			if constexpr (Signed)
				return wasm::_InstSimple{ wasm::_InstSimple::Type::lessSigned, Type };
			else
				return wasm::_InstSimple{ wasm::_InstSimple::Type::lessUnsigned, Type };
		}
		static constexpr wasm::_InstSimple GreaterEqual() {
			if constexpr (Signed)
				return wasm::_InstSimple{ wasm::_InstSimple::Type::greaterEqualSigned, Type };
			else
				return wasm::_InstSimple{ wasm::_InstSimple::Type::greaterEqualUnsigned, Type };
		}
		static constexpr wasm::_InstSimple LessEqual() {
			if constexpr (Signed)
				return wasm::_InstSimple{ wasm::_InstSimple::Type::lessEqualSigned, Type };
			else
				return wasm::_InstSimple{ wasm::_InstSimple::Type::lessEqualUnsigned, Type };
		}
	};

	template <wasm::_OpType Type, bool Signed>
	struct _Arithmetic {
		static constexpr wasm::_InstSimple Add() {
			return wasm::_InstSimple{ wasm::_InstSimple::Type::add, Type };
		}
		static constexpr wasm::_InstSimple Sub() {
			return wasm::_InstSimple{ wasm::_InstSimple::Type::sub, Type };
		}
		static constexpr wasm::_InstSimple Mul() {
			return wasm::_InstSimple{ wasm::_InstSimple::Type::mul, Type };
		}
		static constexpr wasm::_InstSimple Div() {
			if constexpr (Signed)
				return wasm::_InstSimple{ wasm::_InstSimple::Type::divSigned, Type };
			else
				return wasm::_InstSimple{ wasm::_InstSimple::Type::divUnsigned, Type };
		}
		static constexpr wasm::_InstSimple Mod() {
			if constexpr (Signed)
				return wasm::_InstSimple{ wasm::_InstSimple::Type::modSigned, Type };
			else
				return wasm::_InstSimple{ wasm::_InstSimple::Type::modUnsigned, Type };
		}
	};

	template <wasm::_OpType Type>
	struct _SmallConvert {
		static constexpr wasm::_InstSimple Expand() {
			return wasm::_InstSimple{ wasm::_InstSimple::Type::expand, Type };
		}
	};

	template <wasm::_OpType Type>
	struct _LargeConvert {
		static constexpr wasm::_InstSimple Shrink() {
			return wasm::_InstSimple{ wasm::_InstSimple::Type::shrink, Type };
		}
	};

	template <wasm::_OpType Type>
	struct _FloatConvert {
		static constexpr wasm::_InstSimple AsInt() {
			return wasm::_InstSimple{ wasm::_InstSimple::Type::reinterpretAsInt, Type };
		}
	};

	template <wasm::_OpType Type, bool Signed>
	struct _IntConvert {
		static constexpr wasm::_InstSimple ToF32() {
			if constexpr (Signed)
				return wasm::_InstSimple{ wasm::_InstSimple::Type::convertToF32Signed, Type };
			else
				return wasm::_InstSimple{ wasm::_InstSimple::Type::convertToF32Unsigned, Type };
		}
		static constexpr wasm::_InstSimple ToF64() {
			if constexpr (Signed)
				return wasm::_InstSimple{ wasm::_InstSimple::Type::convertToF64Signed, Type };
			else
				return wasm::_InstSimple{ wasm::_InstSimple::Type::convertToF64Unsigned, Type };
		}
		static constexpr wasm::_InstSimple FromF32() {
			if constexpr (Signed)
				return wasm::_InstSimple{ wasm::_InstSimple::Type::convertFromF32Signed, Type };
			else
				return wasm::_InstSimple{ wasm::_InstSimple::Type::convertFromF32Unsigned, Type };
		}
		static constexpr wasm::_InstSimple FromF64() {
			if constexpr (Signed)
				return wasm::_InstSimple{ wasm::_InstSimple::Type::convertFromF64Signed, Type };
			else
				return wasm::_InstSimple{ wasm::_InstSimple::Type::convertFromF64Unsigned, Type };
		}
		static constexpr wasm::_InstSimple AsFloat() {
			return wasm::_InstSimple{ wasm::_InstSimple::Type::reinterpretAsFloat, Type };
		}
	};

	template <wasm::_OpType Type, bool Signed>
	struct _Bitwise {
		static constexpr wasm::_InstSimple And() {
			return wasm::_InstSimple{ wasm::_InstSimple::Type::bitAnd, Type };
		}
		static constexpr wasm::_InstSimple Or() {
			return wasm::_InstSimple{ wasm::_InstSimple::Type::bitOr, Type };
		}
		static constexpr wasm::_InstSimple XOr() {
			return wasm::_InstSimple{ wasm::_InstSimple::Type::bitXOr, Type };
		}
		static constexpr wasm::_InstSimple ShiftLeft() {
			return wasm::_InstSimple{ wasm::_InstSimple::Type::bitShiftLeft, Type };
		}
		static constexpr wasm::_InstSimple ShiftRight() {
			if constexpr (Signed)
				return wasm::_InstSimple{ wasm::_InstSimple::Type::bitShiftRightSigned, Type };
			else
				return wasm::_InstSimple{ wasm::_InstSimple::Type::bitShiftRightUnsigned, Type };
		}
		static constexpr wasm::_InstSimple RotateLeft() {
			return wasm::_InstSimple{ wasm::_InstSimple::Type::bitRotateLeft, Type };
		}
		static constexpr wasm::_InstSimple RotateRight() {
			return wasm::_InstSimple{ wasm::_InstSimple::Type::bitRotateRight, Type };
		}
		static constexpr wasm::_InstSimple LeadingNulls() {
			return wasm::_InstSimple{ wasm::_InstSimple::Type::bitLeadingNulls, Type };
		}
		static constexpr wasm::_InstSimple TrailingNulls() {
			return wasm::_InstSimple{ wasm::_InstSimple::Type::bitTrailingNulls, Type };
		}
		static constexpr wasm::_InstSimple SetBits() {
			return wasm::_InstSimple{ wasm::_InstSimple::Type::bitSetCount, Type };
		}
	};

	template <wasm::_OpType Type>
	struct _Float {
		static constexpr wasm::_InstSimple Min() {
			return wasm::_InstSimple{ wasm::_InstSimple::Type::floatMin, Type };
		}
		static constexpr wasm::_InstSimple Max() {
			return wasm::_InstSimple{ wasm::_InstSimple::Type::floatMax, Type };
		}
		static constexpr wasm::_InstSimple Floor() {
			return wasm::_InstSimple{ wasm::_InstSimple::Type::floatFloor, Type };
		}
		static constexpr wasm::_InstSimple Round() {
			return wasm::_InstSimple{ wasm::_InstSimple::Type::floatRound, Type };
		}
		static constexpr wasm::_InstSimple Ceil() {
			return wasm::_InstSimple{ wasm::_InstSimple::Type::floatCeil, Type };
		}
		static constexpr wasm::_InstSimple Truncate() {
			return wasm::_InstSimple{ wasm::_InstSimple::Type::floatTruncate, Type };
		}
		static constexpr wasm::_InstSimple Absolute() {
			return wasm::_InstSimple{ wasm::_InstSimple::Type::floatAbsolute, Type };
		}
		static constexpr wasm::_InstSimple Negate() {
			return wasm::_InstSimple{ wasm::_InstSimple::Type::floatNegate, Type };
		}
		static constexpr wasm::_InstSimple SquareRoot() {
			return wasm::_InstSimple{ wasm::_InstSimple::Type::floatSquareRoot, Type };
		}
		static constexpr wasm::_InstSimple CopySign() {
			return wasm::_InstSimple{ wasm::_InstSimple::Type::floatCopySign, Type };
		}
	};

	template <wasm::_OpType Type>
	struct _Memory {
		static constexpr wasm::_InstMemory Load(uint32_t offset = 0) {
			return wasm::_InstMemory{ wasm::_InstMemory::Type::load, {}, {}, offset, Type };
		}
		static constexpr wasm::_InstMemory Store(uint32_t offset = 0) {
			return wasm::_InstMemory{ wasm::_InstMemory::Type::store, {}, {}, offset, Type };
		}
		static constexpr wasm::_InstMemory Load(const wasm::_Memory& memory, uint32_t offset = 0) {
			return wasm::_InstMemory{ wasm::_InstMemory::Type::load, memory, {}, offset, Type };
		}
		static constexpr wasm::_InstMemory Store(const wasm::_Memory& memory, uint32_t offset = 0) {
			return wasm::_InstMemory{ wasm::_InstMemory::Type::store, memory, {}, offset, Type };
		}
	};

	template <wasm::_OpType Type, bool Signed>
	struct _LargeMemory {
		static constexpr wasm::_InstMemory Load32(const wasm::_Memory& memory = {}, uint32_t offset = 0) {
			if constexpr (Signed)
				return wasm::_InstMemory{ wasm::_InstMemory::Type::load32Signed, memory, {}, offset, Type };
			else
				return wasm::_InstMemory{ wasm::_InstMemory::Type::load32Unsigned, memory, {}, offset, Type };
		}
		static constexpr wasm::_InstMemory Store32(const wasm::_Memory& memory = {}, uint32_t offset = 0) {
			return wasm::_InstMemory{ wasm::_InstMemory::Type::store32, memory, {}, offset, Type };
		}
	};

	template <wasm::_OpType Type, bool Signed>
	struct _IntMemory {
		static constexpr wasm::_InstMemory Load8(const wasm::_Memory& memory = {}, uint32_t offset = 0) {
			if constexpr (Signed)
				return wasm::_InstMemory{ wasm::_InstMemory::Type::load8Signed, memory, {}, offset, Type };
			else
				return wasm::_InstMemory{ wasm::_InstMemory::Type::load8Unsigned, memory, {}, offset, Type };
		}
		static constexpr wasm::_InstMemory Load16(const wasm::_Memory& memory = {}, uint32_t offset = 0) {
			if constexpr (Signed)
				return wasm::_InstMemory{ wasm::_InstMemory::Type::load16Signed, memory, {}, offset, Type };
			else
				return wasm::_InstMemory{ wasm::_InstMemory::Type::load16Unsigned, memory, {}, offset, Type };
		}
		static constexpr wasm::_InstMemory Store8(const wasm::_Memory& memory = {}, uint32_t offset = 0) {
			return wasm::_InstMemory{ wasm::_InstMemory::Type::store8, memory, {}, offset, Type };
		}
		static constexpr wasm::_InstMemory Store16(const wasm::_Memory& memory = {}, uint32_t offset = 0) {
			return wasm::_InstMemory{ wasm::_InstMemory::Type::store16, memory, {}, offset, Type };
		}
	};
}
