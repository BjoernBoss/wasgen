#pragma once

#include "wasm-instruction.h"

namespace wasm::detail {
	template <wasm::OpType Type, bool Signed>
	struct Constant {
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
	};

	template < wasm::OpType Type, bool Signed>
	struct Compare {
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
	};

	template <wasm::OpType Type, bool Signed>
	struct Arithmetic {
		static constexpr wasm::InstOperand Add() {
			return wasm::InstOperand{ wasm::InstOperand::Type::add, Type };
		}
		static constexpr wasm::InstOperand Sub() {
			return wasm::InstOperand{ wasm::InstOperand::Type::sub, Type };
		}
		static constexpr wasm::InstOperand Mul() {
			return wasm::InstOperand{ wasm::InstOperand::Type::mul, Type };
		}
		static constexpr wasm::InstOperand Div() {
			if constexpr (Signed)
				return wasm::InstOperand{ wasm::InstOperand::Type::divSigned, Type };
			else
				return wasm::InstOperand{ wasm::InstOperand::Type::divUnsigned, Type };
		}
		static constexpr wasm::InstOperand Mod() {
			if constexpr (Signed)
				return wasm::InstOperand{ wasm::InstOperand::Type::modSigned, Type };
			else
				return wasm::InstOperand{ wasm::InstOperand::Type::modUnsigned, Type };
		}
	};

	template <wasm::OpType Type>
	struct SmallConvert {
		static constexpr wasm::InstOperand Expand() {
			return wasm::InstOperand{ wasm::InstOperand::Type::expand, Type };
		}
	};

	template <wasm::OpType Type>
	struct LargeConvert {
		static constexpr wasm::InstOperand Shrink() {
			return wasm::InstOperand{ wasm::InstOperand::Type::shrink, Type };
		}
	};

	template <wasm::OpType Type>
	struct FloatConvert {
		static constexpr wasm::InstOperand AsInt() {
			return wasm::InstOperand{ wasm::InstOperand::Type::reinterpretAsInt, Type };
		}
	};

	template <wasm::OpType Type, bool Signed>
	struct IntConvert {
		static constexpr wasm::InstOperand ToF32() {
			if constexpr (Signed)
				return wasm::InstOperand{ wasm::InstOperand::Type::convertToF32Signed, Type };
			else
				return wasm::InstOperand{ wasm::InstOperand::Type::convertToF32Unsigned, Type };
		}
		static constexpr wasm::InstOperand ToF64() {
			if constexpr (Signed)
				return wasm::InstOperand{ wasm::InstOperand::Type::convertToF64Signed, Type };
			else
				return wasm::InstOperand{ wasm::InstOperand::Type::convertToF64Unsigned, Type };
		}
		static constexpr wasm::InstOperand FromF32() {
			if constexpr (Signed)
				return wasm::InstOperand{ wasm::InstOperand::Type::convertFromF32Signed, Type };
			else
				return wasm::InstOperand{ wasm::InstOperand::Type::convertFromF32Unsigned, Type };
		}
		static constexpr wasm::InstOperand FromF64() {
			if constexpr (Signed)
				return wasm::InstOperand{ wasm::InstOperand::Type::convertFromF64Signed, Type };
			else
				return wasm::InstOperand{ wasm::InstOperand::Type::convertFromF64Unsigned, Type };
		}
		static constexpr wasm::InstOperand AsFloat() {
			return wasm::InstOperand{ wasm::InstOperand::Type::reinterpretAsFloat, Type };
		}
	};

	template <wasm::OpType Type, bool Signed>
	struct Bitwise {
		static constexpr wasm::InstOperand And() {
			return wasm::InstOperand{ wasm::InstOperand::Type::bitAnd, Type };
		}
		static constexpr wasm::InstOperand Or() {
			return wasm::InstOperand{ wasm::InstOperand::Type::bitOr, Type };
		}
		static constexpr wasm::InstOperand XOr() {
			return wasm::InstOperand{ wasm::InstOperand::Type::bitXOr, Type };
		}
		static constexpr wasm::InstOperand ShiftLeft() {
			return wasm::InstOperand{ wasm::InstOperand::Type::bitShiftLeft, Type };
		}
		static constexpr wasm::InstOperand ShiftRight() {
			if constexpr (Signed)
				return wasm::InstOperand{ wasm::InstOperand::Type::bitShiftRightSigned, Type };
			else
				return wasm::InstOperand{ wasm::InstOperand::Type::bitShiftRightUnsigned, Type };
		}
		static constexpr wasm::InstOperand RotateLeft() {
			return wasm::InstOperand{ wasm::InstOperand::Type::bitRotateLeft, Type };
		}
		static constexpr wasm::InstOperand RotateRight() {
			return wasm::InstOperand{ wasm::InstOperand::Type::bitRotateRight, Type };
		}
		static constexpr wasm::InstOperand LeadingNulls() {
			return wasm::InstOperand{ wasm::InstOperand::Type::bitLeadingNulls, Type };
		}
		static constexpr wasm::InstOperand TrailingNulls() {
			return wasm::InstOperand{ wasm::InstOperand::Type::bitTrailingNulls, Type };
		}
		static constexpr wasm::InstOperand SetBits() {
			return wasm::InstOperand{ wasm::InstOperand::Type::bitSetCount, Type };
		}
	};

	template <wasm::OpType Type>
	struct Float {
		static constexpr wasm::InstOperand Min() {
			return wasm::InstOperand{ wasm::InstOperand::Type::floatMin, Type };
		}
		static constexpr wasm::InstOperand Max() {
			return wasm::InstOperand{ wasm::InstOperand::Type::floatMax, Type };
		}
		static constexpr wasm::InstOperand Floor() {
			return wasm::InstOperand{ wasm::InstOperand::Type::floatFloor, Type };
		}
		static constexpr wasm::InstOperand Round() {
			return wasm::InstOperand{ wasm::InstOperand::Type::floatRound, Type };
		}
		static constexpr wasm::InstOperand Ceil() {
			return wasm::InstOperand{ wasm::InstOperand::Type::floatCeil, Type };
		}
		static constexpr wasm::InstOperand Truncate() {
			return wasm::InstOperand{ wasm::InstOperand::Type::floatTruncate, Type };
		}
		static constexpr wasm::InstOperand Absolute() {
			return wasm::InstOperand{ wasm::InstOperand::Type::floatAbsolute, Type };
		}
		static constexpr wasm::InstOperand Negate() {
			return wasm::InstOperand{ wasm::InstOperand::Type::floatNegate, Type };
		}
		static constexpr wasm::InstOperand SquareRoot() {
			return wasm::InstOperand{ wasm::InstOperand::Type::floatSquareRoot, Type };
		}
		static constexpr wasm::InstOperand CopySign() {
			return wasm::InstOperand{ wasm::InstOperand::Type::floatCopySign, Type };
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
