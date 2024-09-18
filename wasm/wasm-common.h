#pragma once

#include <ustring/str.h>
#include <cinttypes>
#include <concepts>
#include <type_traits>
#include <limits>
#include <string>
#include <vector>

namespace wasm {
	/* native types supported by wasm */
	enum class Type : uint8_t {
		i32,
		i64,
		f32,
		f64,
		v128,
		refExtern,
		refFunction
	};

	/* parameter to construct any prototype */
	struct Param {
		std::u8string id;
		wasm::Type type;
		constexpr Param(wasm::Type type, std::u8string id = u8"") : id{ id }, type{ type } {}
	};
	struct Proto {
		std::vector<wasm::Param> params;
		std::vector<wasm::Type> result;
		constexpr Proto() = default;
		constexpr Proto(std::initializer_list<wasm::Param> params, std::initializer_list<wasm::Type> result) : params{ params }, result{ result } {}
	};

	/* operands used by instructions */
	enum class OperandType : uint8_t {
		i32,
		i64,
		f32,
		f64
	};

	/* instruction types using no direct operands */
	enum class NoOpType : uint8_t {
		equal,
		notEqual,
		equalZero,
		greaterSigned,
		greaterUnsigned,
		lessSigned,
		lessUnsigned,
		greaterEqualSigned,
		greaterEqualUnsigned,
		lessEqualSigned,
		lessEqualUnsigned,

		add,
		sub,
		mul,
		divSigned,
		divUnsigned,
		modSigned,
		modUnsigned,

		convertToF32Signed,
		convertToF32Unsigned,
		convertToF64Signed,
		convertToF64Unsigned,
		convertFromF32Signed,
		convertFromF32Unsigned,
		convertFromF64Signed,
		convertFromF64Unsigned,

		reinterpretAsInt,
		reinterpretAsFloat,

		expand,
		shrink,

		bitAnd,
		bitOr,
		bitXOr,
		bitShiftLeft,
		bitShiftRightSigned,
		bitShiftRightUnsigned,
		bitRotateLeft,
		bitRotateRight,
		bitLeadingNulls,
		bitTrailingNulls,
		bitSetCount,

		floatMin,
		floatMax,
		floatFloor,
		floatRound,
		floatCeil,
		floatTruncate,
		floatAbsolute,
		floatNegate,
		floatSquareRoot,
		floatCopySigne
	};

	/* instruction types referencing memory */
	enum class MemOpType : uint8_t {
		load,
		load8Unsigned,
		load8Signed,
		load16Unsigned,
		load16Signed,
		load32Unsigned,
		load32Signed,
		store,
		store8,
		store16,
		store32,

		grow,
		size,
		copy,
		fill
	};

	/* instruction types referencing local/global variables */
	enum class VarOpType : uint8_t {
		set,
		get,
		tee
	};
}
