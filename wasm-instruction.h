/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2024 Bjoern Boss Henrichsen */
#pragma once

#include "wasm-memory.h"
#include "wasm-table.h"
#include "wasm-global.h"
#include "wasm-prototype.h"
#include "wasm-function.h"
#include "wasm-variable.h"
#include "wasm-target.h"

namespace wasm {
	/* supported operand types */
	enum class OpType : uint8_t {
		i32,
		i64,
		f32,
		f64
	};

	/* description of any simple instructions, which do not take any direct operands */
	struct InstSimple {
	public:
		enum class Type : uint8_t {
			drop,
			nop,
			ret,
			unreachable,
			select,
			selectRefFunction,
			selectRefExtern,
			refTestNull,
			refNullFunction,
			refNullExtern,
			expandIntSigned,
			expandIntUnsigned,
			shrinkInt,
			expandFloat,
			shrinkFloat
		};

	public:
		Type type = Type::nop;

	public:
		constexpr InstSimple(Type type) : type{ type } {}
	};

	/* description of any simple instructions, which take a single constant as operand */
	struct InstConst {
	public:
		std::variant<uint32_t, uint64_t, float, double> value;

	public:
		constexpr InstConst(uint32_t value) : value{ value } {}
		constexpr InstConst(uint64_t value) : value{ value } {}
		constexpr InstConst(float value) : value{ value } {}
		constexpr InstConst(double value) : value{ value } {}
	};

	/* description of any simple instructions, which only require an operation-type as operand */
	struct InstOperand {
	public:
		enum class Type : uint8_t {
			equal,
			notEqual,
			add,
			sub,
			mul
		};

	public:
		Type type = Type::equal;
		wasm::OpType operand = wasm::OpType::i32;

	public:
		constexpr InstOperand(Type type, wasm::OpType operand) : type{ type }, operand{ operand } {}
	};

	/* description of any simple instructions, which operate either on the 32 or 64 version of a type */
	struct InstWidth {
	public:
		enum class Type : uint8_t {
			equalZero,
			greater,
			less,
			greaterEqual,
			lessEqual,
			greaterSigned,
			greaterUnsigned,
			lessSigned,
			lessUnsigned,
			greaterEqualSigned,
			greaterEqualUnsigned,
			lessEqualSigned,
			lessEqualUnsigned,
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
			reinterpretAsFloat,
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
			floatDiv,
			reinterpretAsInt,
			floatMin,
			floatMax,
			floatFloor,
			floatRound,
			floatCeil,
			floatTruncate,
			floatAbsolute,
			floatNegate,
			floatSquareRoot,
			floatCopySign
		};

	public:
		Type type = Type::divSigned;
		bool width32 = false;

	public:
		constexpr InstWidth(Type type, bool width32) : type{ type }, width32{ width32 } {}
	};

	/* description of any memory-interacting instructions */
	struct InstMemory {
	public:
		enum class Type : uint8_t {
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

	public:
		wasm::Memory memory;
		wasm::Memory destination;
		uint32_t offset = 0;
		Type type = Type::load;
		wasm::OpType operand = OpType::i32;

	public:
		constexpr InstMemory(Type type, const wasm::Memory& memory, const wasm::Memory& destination, uint32_t offset, wasm::OpType operand) : memory{ memory }, destination{ destination }, offset{ offset }, type{ type }, operand{ operand } {}
	};

	/* description of any table-interacting instructions */
	struct InstTable {
	public:
		enum class Type : uint8_t {
			get,
			set,
			size,
			grow,
			fill,
			copy
		};

	public:
		wasm::Table table;
		wasm::Table destination;
		Type type = Type::get;

	public:
		constexpr InstTable(Type type, const wasm::Table& table, const wasm::Table& destination) : table{ table }, destination{ destination }, type{ type } {}
	};

	/* description of any local variable-interacting instructions */
	struct InstLocal {
	public:
		enum class Type : uint8_t {
			set,
			get,
			tee
		};

	public:
		wasm::Variable variable;
		Type type = Type::set;

	public:
		constexpr InstLocal(Type type, const wasm::Variable& variable) : variable{ variable }, type{ type } {}
	};

	/* description of any global variable-interacting instructions */
	struct InstGlobal {
	public:
		enum class Type : uint8_t {
			set,
			get
		};

	public:
		wasm::Global global;
		Type type = Type::set;

	public:
		constexpr InstGlobal(Type type, const wasm::Global& global) : global{ global }, type{ type } {}
	};

	/* description of any function-interacting instructions */
	struct InstFunction {
	public:
		enum class Type : uint8_t {
			refFunction,
			callNormal,
			callTail
		};

	public:
		wasm::Function function;
		Type type = Type::refFunction;

	public:
		constexpr InstFunction(Type type, const wasm::Function& function) : function{ function }, type{ type } {}
	};

	/* description of any indirect-call instructions */
	struct InstIndirect {
	public:
		enum class Type : uint8_t {
			callNormal,
			callTail
		};

	public:
		wasm::Table table;
		wasm::Prototype prototype;
		Type type = Type::callNormal;

	public:
		constexpr InstIndirect(Type type, const wasm::Table& table, const wasm::Prototype& prototype) : table{ table }, prototype{ prototype }, type{ type } {}
	};

	/* wrapper necessary to be able to create initializer-lists from non-copyable targets */
	struct WTarget {
		const wasm::Target& target;
		constexpr WTarget(const wasm::Target& target) : target{ target } {}
		constexpr operator const wasm::Target& () const {
			return target;
		}
		const wasm::Target& get() const {
			return target;
		}
	};

	/* description of any branch instructions */
	struct InstBranch {
	public:
		enum class Type : uint8_t {
			direct,
			conditional,
			table
		};

	public:
		std::initializer_list<wasm::WTarget> list;
		const wasm::Target& target;
		Type type = Type::direct;

	public:
		constexpr InstBranch(Type type, std::initializer_list<wasm::WTarget> list, const wasm::Target& target) : list(list), target{ target }, type{ type } {}
	};
}
