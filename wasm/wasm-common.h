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
		constexpr Param(std::u8string id, wasm::Type type) : id{ id }, type{ type } {}
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
		floatCopySign,

		drop,
		nop,
		ret,
		unreachable,
		select,
		selectRefFunction,
		selectRefExtern
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

	/* instruction types referencing global variables */
	enum class GlobOpType : uint8_t {
		set,
		get
	};

	/* instruction types referencing local variables */
	enum class LocOpType : uint8_t {
		set,
		get,
		tee
	};

	/* instruction types referencing tables */
	enum class TabOpType : uint8_t {
		get,
		set,
		size,
		grow,
		fill,
		copy
	};

	/* instruction types referencing references */
	enum class RefOpType : uint8_t {
		testNull,
		nullFunction,
		nullExtern,
		function
	};

	/* instruction types referencing function calls */
	enum class CallOpType : uint8_t {
		normal,
		tail
	};

	/* instruction types referencing branch instructions */
	enum class BrOpType : uint8_t {
		direct,
		conditional,
		table
	};
}
