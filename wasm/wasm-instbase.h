#pragma once

#include "wasm-instruction.h"

namespace wasm::detail {
	template <wasm::OpType Type, bool Signed>
	struct Common {
		template <class ValType>
		static constexpr wasm::InstConst Const(ValType val) {
			if constexpr (Type == wasm::OpType::i32) {
				if constexpr (Signed)
					return wasm::InstConst{ uint32_t(int32_t(val)) };
				return wasm::InstConst{ uint32_t(val) };
			}
			else if constexpr (Type == wasm::OpType::i64) {
				if constexpr (Signed)
					return wasm::InstConst{ uint64_t(int64_t(val)) };
				return wasm::InstConst{ uint64_t(val) };
			}
			else if constexpr (Type == wasm::OpType::f32)
				return wasm::InstConst{ float(val) };
			else
				return wasm::InstConst{ double(val) };
		}
		static constexpr wasm::InstOperand Equal() {
			return wasm::InstOperand{ wasm::InstOperand::Type::equal, Type };
		}
		static constexpr wasm::InstOperand EqualZero() {
			return wasm::InstOperand{ wasm::InstOperand::Type::equalZero, Type };
		}
		static constexpr wasm::InstOperand NotEqual() {
			return wasm::InstOperand{ wasm::InstOperand::Type::notEqual, Type };
		}
		static constexpr wasm::InstOperand Greater() {
			if constexpr (Signed)
				return wasm::InstOperand{ wasm::InstOperand::Type::greaterSigned, Type };
			else
				return wasm::InstOperand{ wasm::InstOperand::Type::greaterUnsigned, Type };
		}
		static constexpr wasm::InstOperand Less() {
			if constexpr (Signed)
				return wasm::InstOperand{ wasm::InstOperand::Type::lessSigned, Type };
			else
				return wasm::InstOperand{ wasm::InstOperand::Type::lessUnsigned, Type };
		}
		static constexpr wasm::InstOperand GreaterEqual() {
			if constexpr (Signed)
				return wasm::InstOperand{ wasm::InstOperand::Type::greaterEqualSigned, Type };
			else
				return wasm::InstOperand{ wasm::InstOperand::Type::greaterEqualUnsigned, Type };
		}
		static constexpr wasm::InstOperand LessEqual() {
			if constexpr (Signed)
				return wasm::InstOperand{ wasm::InstOperand::Type::lessEqualSigned, Type };
			else
				return wasm::InstOperand{ wasm::InstOperand::Type::lessEqualUnsigned, Type };
		}
		static constexpr wasm::InstOperand Add() {
			return wasm::InstOperand{ wasm::InstOperand::Type::add, Type };
		}
		static constexpr wasm::InstOperand Sub() {
			return wasm::InstOperand{ wasm::InstOperand::Type::sub, Type };
		}
		static constexpr wasm::InstOperand Mul() {
			return wasm::InstOperand{ wasm::InstOperand::Type::mul, Type };
		}
	};

	template <bool Float, bool Signed>
	struct SmallConvert {
		static constexpr wasm::InstSimple Expand() {
			if constexpr (Float)
				return wasm::InstSimple{ wasm::InstSimple::Type::expandFloat };
			else if constexpr (Signed)
				return wasm::InstSimple{ wasm::InstSimple::Type::expandIntSigned };
			else
				return wasm::InstSimple{ wasm::InstSimple::Type::expandIntUnsigned };
		}
	};

	template <bool Float>
	struct LargeConvert {
		static constexpr wasm::InstSimple Shrink() {
			if constexpr (Float)
				return wasm::InstSimple{ wasm::InstSimple::Type::shrinkFloat };
			else
				return wasm::InstSimple{ wasm::InstSimple::Type::shrinkInt };
		}
	};

	template <bool I32, bool Signed>
	struct IntOperations {
		static constexpr wasm::InstWidth Div() {
			if constexpr (Signed)
				return wasm::InstWidth{ wasm::InstWidth::Type::divSigned, I32 };
			else
				return wasm::InstWidth{ wasm::InstWidth::Type::divUnsigned, I32 };
		}
		static constexpr wasm::InstWidth Mod() {
			if constexpr (Signed)
				return wasm::InstWidth{ wasm::InstWidth::Type::modSigned, I32 };
			else
				return wasm::InstWidth{ wasm::InstWidth::Type::modUnsigned, I32 };
		}
		static constexpr wasm::InstWidth ToF32() {
			if constexpr (Signed)
				return wasm::InstWidth{ wasm::InstWidth::Type::convertToF32Signed, I32 };
			else
				return wasm::InstWidth{ wasm::InstWidth::Type::convertToF32Unsigned, I32 };
		}
		static constexpr wasm::InstWidth ToF64() {
			if constexpr (Signed)
				return wasm::InstWidth{ wasm::InstWidth::Type::convertToF64Signed, I32 };
			else
				return wasm::InstWidth{ wasm::InstWidth::Type::convertToF64Unsigned, I32 };
		}
		static constexpr wasm::InstWidth FromF32() {
			if constexpr (Signed)
				return wasm::InstWidth{ wasm::InstWidth::Type::convertFromF32Signed, I32 };
			else
				return wasm::InstWidth{ wasm::InstWidth::Type::convertFromF32Unsigned, I32 };
		}
		static constexpr wasm::InstWidth FromF64() {
			if constexpr (Signed)
				return wasm::InstWidth{ wasm::InstWidth::Type::convertFromF64Signed, I32 };
			else
				return wasm::InstWidth{ wasm::InstWidth::Type::convertFromF64Unsigned, I32 };
		}
		static constexpr wasm::InstWidth AsFloat() {
			return wasm::InstWidth{ wasm::InstWidth::Type::reinterpretAsFloat, I32 };
		}
		static constexpr wasm::InstWidth And() {
			return wasm::InstWidth{ wasm::InstWidth::Type::bitAnd, I32 };
		}
		static constexpr wasm::InstWidth Or() {
			return wasm::InstWidth{ wasm::InstWidth::Type::bitOr, I32 };
		}
		static constexpr wasm::InstWidth XOr() {
			return wasm::InstWidth{ wasm::InstWidth::Type::bitXOr, I32 };
		}
		static constexpr wasm::InstWidth ShiftLeft() {
			return wasm::InstWidth{ wasm::InstWidth::Type::bitShiftLeft, I32 };
		}
		static constexpr wasm::InstWidth ShiftRight() {
			if constexpr (Signed)
				return wasm::InstWidth{ wasm::InstWidth::Type::bitShiftRightSigned, I32 };
			else
				return wasm::InstWidth{ wasm::InstWidth::Type::bitShiftRightUnsigned, I32 };
		}
		static constexpr wasm::InstWidth RotateLeft() {
			return wasm::InstWidth{ wasm::InstWidth::Type::bitRotateLeft, I32 };
		}
		static constexpr wasm::InstWidth RotateRight() {
			return wasm::InstWidth{ wasm::InstWidth::Type::bitRotateRight, I32 };
		}
		static constexpr wasm::InstWidth LeadingNulls() {
			return wasm::InstWidth{ wasm::InstWidth::Type::bitLeadingNulls, I32 };
		}
		static constexpr wasm::InstWidth TrailingNulls() {
			return wasm::InstWidth{ wasm::InstWidth::Type::bitTrailingNulls, I32 };
		}
		static constexpr wasm::InstWidth SetBits() {
			return wasm::InstWidth{ wasm::InstWidth::Type::bitSetCount, I32 };
		}
	};

	template <bool F32>
	struct FloatOperations {
		static constexpr wasm::InstWidth Div() {
			return wasm::InstWidth{ wasm::InstWidth::Type::floatDiv, F32 };
		}
		static constexpr wasm::InstWidth AsInt() {
			return wasm::InstWidth{ wasm::InstWidth::Type::reinterpretAsInt, F32 };
		}
		static constexpr wasm::InstWidth Min() {
			return wasm::InstWidth{ wasm::InstWidth::Type::floatMin, F32 };
		}
		static constexpr wasm::InstWidth Max() {
			return wasm::InstWidth{ wasm::InstWidth::Type::floatMax, F32 };
		}
		static constexpr wasm::InstWidth Floor() {
			return wasm::InstWidth{ wasm::InstWidth::Type::floatFloor, F32 };
		}
		static constexpr wasm::InstWidth Round() {
			return wasm::InstWidth{ wasm::InstWidth::Type::floatRound, F32 };
		}
		static constexpr wasm::InstWidth Ceil() {
			return wasm::InstWidth{ wasm::InstWidth::Type::floatCeil, F32 };
		}
		static constexpr wasm::InstWidth Truncate() {
			return wasm::InstWidth{ wasm::InstWidth::Type::floatTruncate, F32 };
		}
		static constexpr wasm::InstWidth Absolute() {
			return wasm::InstWidth{ wasm::InstWidth::Type::floatAbsolute, F32 };
		}
		static constexpr wasm::InstWidth Negate() {
			return wasm::InstWidth{ wasm::InstWidth::Type::floatNegate, F32 };
		}
		static constexpr wasm::InstWidth SquareRoot() {
			return wasm::InstWidth{ wasm::InstWidth::Type::floatSquareRoot, F32 };
		}
		static constexpr wasm::InstWidth CopySign() {
			return wasm::InstWidth{ wasm::InstWidth::Type::floatCopySign, F32 };
		}
	};

	template <wasm::OpType Type>
	struct Memory {
		static constexpr wasm::InstMemory Load(const wasm::Memory& memory = {}, uint32_t offset = 0) {
			return wasm::InstMemory{ wasm::InstMemory::Type::load, memory, {}, offset, Type };
		}
		static constexpr wasm::InstMemory Store(const wasm::Memory& memory = {}, uint32_t offset = 0) {
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
