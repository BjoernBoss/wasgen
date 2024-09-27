#pragma once

#include "_wasm-memory.h"
#include "_wasm-table.h"
#include "_wasm-global.h"
#include "_wasm-prototype.h"
#include "_wasm-function.h"
#include "_wasm-variable.h"
#include "_wasm-target.h"

namespace wasm {
	enum class _OpType : uint8_t {
		i32,
		i64,
		f32,
		f64
	};

	struct _InstConst {
	public:
		std::variant<uint32_t, uint64_t, float, double> value;
		wasm::_OpType operand = wasm::_OpType::i32;

	public:
		constexpr _InstConst(uint32_t value, wasm::_OpType operand) : value{ value }, operand{ operand } {}
		constexpr _InstConst(uint64_t value, wasm::_OpType operand) : value{ value }, operand{ operand } {}
		constexpr _InstConst(float value, wasm::_OpType operand) : value{ value }, operand{ operand } {}
		constexpr _InstConst(double value, wasm::_OpType operand) : value{ value }, operand{ operand } {}
	};

	struct _InstSimple {
	public:
		enum class Type : uint8_t {
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
			selectRefExtern,

			refTestNull,
			refNullFunction,
			refNullExtern
		};

	public:
		Type type = Type::nop;
		wasm::_OpType operand = wasm::_OpType::i32;

	public:
		constexpr _InstSimple(Type type, wasm::_OpType operand) : type{ type }, operand{ operand } {}
	};

	struct _InstMemory {
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
		wasm::_Memory memory;
		wasm::_Memory source;
		uint32_t offset = 0;
		Type type = Type::load;
		wasm::_OpType operand = _OpType::i32;

	public:
		constexpr _InstMemory(Type type, const wasm::_Memory& memory, const wasm::_Memory& source, uint32_t offset, wasm::_OpType operand) : memory{ memory }, source{ source }, offset{ offset }, type{ type }, operand{ operand } {}
	};

	struct _InstTable {
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
		wasm::_Table table;
		wasm::_Table source;
		Type type = Type::get;

	public:
		constexpr _InstTable(Type type, const wasm::_Table& table, const wasm::_Table& source) : table{ table }, source{ source }, type{ type } {}
	};

	struct _InstLocal {
	public:
		enum class Type : uint8_t {
			set,
			get,
			tee
		};

	public:
		wasm::_Variable variable;
		Type type = Type::set;

	public:
		constexpr _InstLocal(Type type, const wasm::_Variable& variable) : variable{ variable }, type{ type } {}
	};

	struct _InstGlobal {
	public:
		enum class Type : uint8_t {
			set,
			get
		};

	public:
		wasm::_Global global;
		Type type = Type::set;

	public:
		constexpr _InstGlobal(Type type, const wasm::_Global& global) : global{ global }, type{ type } {}
	};

	struct _InstFunction {
	public:
		enum class Type : uint8_t {
			refFunction,
			callNormal,
			callTail
		};

	public:
		wasm::_Function function;
		Type type = Type::refFunction;

	public:
		constexpr _InstFunction(Type type, const wasm::_Function& function) : function{ function }, type{ type } {}
	};

	struct _InstIndirect {
	public:
		enum class Type : uint8_t {
			callNormal,
			callTail
		};

	public:
		wasm::_Table table;
		wasm::_Prototype prototype;
		Type type = Type::callNormal;

	public:
		constexpr _InstIndirect(Type type, const wasm::_Table& table, const wasm::_Prototype& prototype) : table{ table }, prototype{ prototype }, type{ type } {}
	};

	/* wrapper necessary to be able to create initializer-lists from non-copyable targets */
	struct _WTarget {
		const wasm::_Target& target;
		constexpr _WTarget(const wasm::_Target& target) : target{ target } {}
		constexpr operator const wasm::_Target& () const {
			return target;
		}
	};

	struct _InstBranch {
	public:
		enum class Type : uint8_t {
			direct,
			conditional,
			table
		};

	public:
		std::initializer_list<wasm::_WTarget> list;
		const wasm::_Target& target;
		Type type = Type::direct;

	public:
		constexpr _InstBranch(Type type, std::initializer_list<wasm::_WTarget> list, const wasm::_Target& target) : list(list), target{ target }, type{ type } {}
	};
}
