#pragma once

#include "wasm-common.h"
#include "wasm-types.h"

namespace wasm {
	/* writer must provide a type Inst, which must provide static
	*	functions for all of the separate instruction functions */
	template <class Type>
	concept IsInst = requires() {
		typename Type::Inst;
		{ Type::Inst::Consti32(0lu) } -> std::same_as<typename Type::Instruction>;
		{ Type::Inst::Consti64(0llu) } -> std::same_as<typename Type::Instruction>;
		{ Type::Inst::Constf32(0.0f) } -> std::same_as<typename Type::Instruction>;
		{ Type::Inst::Constf64(0.0) } -> std::same_as<typename Type::Instruction>;

		{ Type::Inst::NoOp(wasm::NoOpType::equal, wasm::OperandType::i32) } -> std::same_as<typename Type::Instruction>;
	};
}

namespace inst::detail {
	template <class Writer, wasm::OperandType Type, bool Signed>
	struct Constant {
		template <class ValType>
		static constexpr wasm::Instruction<Writer> Const(ValType val) {
			if constexpr (Type == wasm::OperandType::i32) {
				if constexpr (Signed)
					return wasm::Instruction<Writer>{ std::move(Writer::Inst::Consti32(uint32_t(int32_t(val)))) };
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::Consti32(uint32_t(val))) };
			}
			else if constexpr (Type == wasm::OperandType::i64) {
				if constexpr (Signed)
					return wasm::Instruction<Writer>{ std::move(Writer::Inst::Consti64(uint64_t(int64_t(val)))) };
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::Consti64(uint64_t(val))) };
			}
			else if constexpr (Type == wasm::OperandType::f32)
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::Constf32(float(val))) };
			else
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::Constf64(double(val))) };
		}
	};

	template <class Writer, wasm::OperandType Type, bool Signed>
	struct Compare {
		static constexpr wasm::Instruction<Writer> Equal() {
			return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::equal, Type)) };
		}
		static constexpr wasm::Instruction<Writer> EqualZero() {
			return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::equalZero, Type)) };
		}
		static constexpr wasm::Instruction<Writer> NotEqual() {
			return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::notEqual, Type)) };
		}
		static constexpr wasm::Instruction<Writer> Greater() {
			if constexpr (Signed)
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::greaterSigned, Type)) };
			else
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::greaterUnsigned, Type)) };
		}
		static constexpr wasm::Instruction<Writer> Less() {
			if constexpr (Signed)
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::lessSigned, Type)) };
			else
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::lessUnsigned, Type)) };
		}
		static constexpr wasm::Instruction<Writer> GreaterEqual() {
			if constexpr (Signed)
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::greaterEqualSigned, Type)) };
			else
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::greaterEqualUnsigned, Type)) };
		}
		static constexpr wasm::Instruction<Writer> LessEqual() {
			if constexpr (Signed)
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::lessEqualSigned, Type)) };
			else
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::lessEqualUnsigned, Type)) };
		}
	};

	template <class Writer, wasm::OperandType Type, bool Signed>
	struct Arithmetic {
		static constexpr wasm::Instruction<Writer> Add() {
			return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::add, Type)) };
		}
		static constexpr wasm::Instruction<Writer> Sub() {
			return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::sub, Type)) };
		}
		static constexpr wasm::Instruction<Writer> Mul() {
			return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::mul, Type)) };
		}
		static constexpr wasm::Instruction<Writer> Div() {
			if constexpr (Signed)
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::divSigned, Type)) };
			else
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::divUnsigned, Type)) };
		}
		static constexpr wasm::Instruction<Writer> Mod() {
			if constexpr (Signed)
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::modSigned, Type)) };
			else
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::modUnsigned, Type)) };
		}
	};

	template <class Writer, wasm::OperandType Type>
	struct SmallConvert {
		static constexpr wasm::Instruction<Writer> Expand() {
			return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::expand, Type)) };
		}
	};

	template <class Writer, wasm::OperandType Type>
	struct LargeConvert {
		static constexpr wasm::Instruction<Writer> Shrink() {
			return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::shrink, Type)) };
		}
	};

	template <class Writer, wasm::OperandType Type>
	struct FloatConvert {
		static constexpr wasm::Instruction<Writer> AsInt() {
			return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::reinterpretAsInt, Type)) };
		}
	};

	template <class Writer, wasm::OperandType Type, bool Signed>
	struct IntConvert {
		static constexpr wasm::Instruction<Writer> ToF32() {
			if constexpr (Signed)
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::convertToF32Signed, Type)) };
			else
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::convertToF32Unsigned, Type)) };
		}
		static constexpr wasm::Instruction<Writer> ToF64() {
			if constexpr (Signed)
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::convertToF64Signed, Type)) };
			else
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::convertToF64Unsigned, Type)) };
		}
		static constexpr wasm::Instruction<Writer> FromF32() {
			if constexpr (Signed)
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::convertFromF32Signed, Type)) };
			else
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::convertFromF32Unsigned, Type)) };
		}
		static constexpr wasm::Instruction<Writer> FromF64() {
			if constexpr (Signed)
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::convertFromF64Signed, Type)) };
			else
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::convertFromF64Unsigned, Type)) };
		}
		static constexpr wasm::Instruction<Writer> AsFloat() {
			return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::reinterpretAsFloat, Type)) };
		}
	};

	template <class Writer, wasm::OperandType Type, bool Signed>
	struct Bitwise {
		static constexpr wasm::Instruction<Writer> And() {
			return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::bitAnd, Type)) };
		}
		static constexpr wasm::Instruction<Writer> Or() {
			return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::bitOr, Type)) };
		}
		static constexpr wasm::Instruction<Writer> XOr() {
			return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::bitXOr, Type)) };
		}
		static constexpr wasm::Instruction<Writer> ShiftLeft() {
			return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::bitShiftLeft, Type)) };
		}
		static constexpr wasm::Instruction<Writer> ShiftRight() {
			if constexpr (Signed)
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::bitShiftRightSigned, Type)) };
			else
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::bitShiftRightUnsigned, Type)) };
		}
		static constexpr wasm::Instruction<Writer> RotateLeft() {
			return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::bitRotateLeft, Type)) };
		}
		static constexpr wasm::Instruction<Writer> RotateRight() {
			return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::bitRotateRight, Type)) };
		}
		static constexpr wasm::Instruction<Writer> LeadingNulls() {
			return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::bitLeadingNulls, Type)) };
		}
		static constexpr wasm::Instruction<Writer> TrailingNulls() {
			return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::bitTrailingNulls, Type)) };
		}
		static constexpr wasm::Instruction<Writer> SetBits() {
			return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::bitSetCount, Type)) };
		}

	};

	template <class Writer, wasm::OperandType Type>
	struct Float {
		static constexpr wasm::Instruction<Writer> Min() {
			return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::floatMin, Type)) };
		}
		static constexpr wasm::Instruction<Writer> Max() {
			return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::floatMax, Type)) };
		}
		static constexpr wasm::Instruction<Writer> Floor() {
			return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::floatFloor, Type)) };
		}
		static constexpr wasm::Instruction<Writer> Round() {
			return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::floatRound, Type)) };
		}
		static constexpr wasm::Instruction<Writer> Ceil() {
			return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::floatCeil, Type)) };
		}
		static constexpr wasm::Instruction<Writer> Truncate() {
			return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::floatTruncate, Type)) };
		}
		static constexpr wasm::Instruction<Writer> Absolute() {
			return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::floatAbsolute, Type)) };
		}
		static constexpr wasm::Instruction<Writer> Negate() {
			return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::floatNegate, Type)) };
		}
		static constexpr wasm::Instruction<Writer> SquareRoot() {
			return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::floatSquareRoot, Type)) };
		}
		static constexpr wasm::Instruction<Writer> CopySign() {
			return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::floatCopySign, Type)) };
		}
	};

	template <class Writer, wasm::OperandType Type>
	struct Memory {
		static constexpr wasm::Instruction<Writer> Load(uint32_t offset = 0, const wasm::Memory<Writer>* memory = 0) {
			return wasm::Instruction<Writer>{ std::move(Writer::Inst::Memory(memory, offset, wasm::MemOpType::load, Type)) };
		}
		static constexpr wasm::Instruction<Writer> Store(uint32_t offset = 0, const wasm::Memory<Writer>* memory = 0) {
			return wasm::Instruction<Writer>{ std::move(Writer::Inst::Memory(memory, offset, wasm::MemOpType::store, Type)) };
		}
	};

	template <class Writer, wasm::OperandType Type, bool Signed>
	struct LargeMemory {
		static constexpr wasm::Instruction<Writer> Load32(uint32_t offset = 0, const wasm::Memory<Writer>* memory = 0) {
			if constexpr (Signed)
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::Memory(memory, offset, wasm::MemOpType::load32Signed, Type)) };
			else
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::Memory(memory, offset, wasm::MemOpType::load32Unsigned, Type)) };
		}
		static constexpr wasm::Instruction<Writer> Store32(uint32_t offset = 0, const wasm::Memory<Writer>* memory = 0) {
			return wasm::Instruction<Writer>{ std::move(Writer::Inst::Memory(memory, offset, wasm::MemOpType::store32, Type)) };
		}
	};

	template <class Writer, wasm::OperandType Type, bool Signed>
	struct IntMemory {
		static constexpr wasm::Instruction<Writer> Load8(uint32_t offset = 0, const wasm::Memory<Writer>* memory = 0) {
			if constexpr (Signed)
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::Memory(memory, offset, wasm::MemOpType::load8Signed, Type)) };
			else
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::Memory(memory, offset, wasm::MemOpType::load8Unsigned, Type)) };
		}
		static constexpr wasm::Instruction<Writer> Load16(uint32_t offset = 0, const wasm::Memory<Writer>* memory = 0) {
			if constexpr (Signed)
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::Memory(memory, offset, wasm::MemOpType::load16Signed, Type)) };
			else
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::Memory(memory, offset, wasm::MemOpType::load16Unsigned, Type)) };
		}
		static constexpr wasm::Instruction<Writer> Store8(uint32_t offset = 0, const wasm::Memory<Writer>* memory = 0) {
			return wasm::Instruction<Writer>{ std::move(Writer::Inst::Memory(memory, offset, wasm::MemOpType::store8, Type)) };
		}
		static constexpr wasm::Instruction<Writer> Store16(uint32_t offset = 0, const wasm::Memory<Writer>* memory = 0) {
			return wasm::Instruction<Writer>{ std::move(Writer::Inst::Memory(memory, offset, wasm::MemOpType::store16, Type)) };
		}
	};
}
