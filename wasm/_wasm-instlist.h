#pragma once

#include "_wasm-instbase.h"

namespace wasm::inst {
	struct Branch {
		static constexpr wasm::_InstBranch Direct(const wasm::_Target& target) {
			return wasm::_InstBranch{ wasm::_InstBranch::Type::direct, {}, target };
		}
		static constexpr wasm::_InstBranch If(const wasm::_Target& target) {
			return wasm::_InstBranch{ wasm::_InstBranch::Type::conditional, {}, target };
		}
		static constexpr wasm::_InstBranch Table(std::initializer_list<wasm::_WTarget> optTarget, const wasm::_Target& defTarget) {
			return wasm::_InstBranch{ wasm::_InstBranch::Type::table, optTarget, defTarget };
		}
	};

	struct Call {
		static constexpr wasm::_InstFunction Direct(const wasm::_Function& fn) {
			return wasm::_InstFunction{ wasm::_InstFunction::Type::callNormal, fn };
		}
		static constexpr wasm::_InstFunction Tail(const wasm::_Function& fn) {
			return wasm::_InstFunction{ wasm::_InstFunction::Type::callTail, fn };
		}
		static constexpr wasm::_InstIndirect Indirect(const wasm::_Table& table = {}, const wasm::_Prototype& type = {}) {
			return wasm::_InstIndirect{ wasm::_InstIndirect::Type::callNormal, table, type };
		}
		static constexpr wasm::_InstIndirect IndirectTail(const wasm::_Table& table = {}, const wasm::_Prototype& type = {}) {
			return wasm::_InstIndirect{ wasm::_InstIndirect::Type::callTail, table, type };
		}
	};

	struct Local {
		static constexpr wasm::_InstLocal Get(const wasm::_Variable& local) {
			return wasm::_InstLocal{ wasm::_InstLocal::Type::get, local };
		}
		static constexpr wasm::_InstLocal Set(const wasm::_Variable& local) {
			return wasm::_InstLocal{ wasm::_InstLocal::Type::set, local };
		}
		static constexpr wasm::_InstLocal Tee(const wasm::_Variable& local) {
			return wasm::_InstLocal{ wasm::_InstLocal::Type::tee, local };
		}
	};

	struct Global {
		static constexpr wasm::_InstGlobal Get(const wasm::_Global& global) {
			return wasm::_InstGlobal{ wasm::_InstGlobal::Type::get, global };
		}
		static constexpr wasm::_InstGlobal Set(const wasm::_Global& global) {
			return wasm::_InstGlobal{ wasm::_InstGlobal::Type::set, global };
		}
	};

	struct Memory {
		static constexpr wasm::_InstMemory Grow(const wasm::_Memory& memory = {}) {
			return wasm::_InstMemory{ wasm::_InstMemory::Type::grow, memory, {}, 0, wasm::_OpType::i32 };
		}
		static constexpr wasm::_InstMemory Size(const wasm::_Memory& memory = {}) {
			return wasm::_InstMemory{ wasm::_InstMemory::Type::size, memory, {}, 0, wasm::_OpType::i32 };
		}
		static constexpr wasm::_InstMemory Fill(const wasm::_Memory& memory = {}) {
			return wasm::_InstMemory{ wasm::_InstMemory::Type::fill, memory, {}, 0, wasm::_OpType::i32 };
		}
		static constexpr wasm::_InstMemory Copy(const wasm::_Memory& dest = {}, const wasm::_Memory& source = {}) {
			return wasm::_InstMemory{ wasm::_InstMemory::Type::copy, dest, source, 0, wasm::_OpType::i32 };
		}
	};

	struct Table {
		static constexpr wasm::_InstTable Get(const wasm::_Table& table = {}) {
			return wasm::_InstTable{ wasm::_InstTable::Type::get, table, {} };
		}
		static constexpr wasm::_InstTable Set(const wasm::_Table& table = {}) {
			return wasm::_InstTable{ wasm::_InstTable::Type::set, table, {} };
		}
		static constexpr wasm::_InstTable Size(const wasm::_Table& table = {}) {
			return wasm::_InstTable{ wasm::_InstTable::Type::size, table, {} };
		}
		static constexpr wasm::_InstTable Grow(const wasm::_Table& table = {}) {
			return wasm::_InstTable{ wasm::_InstTable::Type::grow, table, {} };
		}
		static constexpr wasm::_InstTable Fill(const wasm::_Table& table = {}) {
			return wasm::_InstTable{ wasm::_InstTable::Type::fill, table, {} };
		}
		static constexpr wasm::_InstTable Copy(const wasm::_Table& dest = {}, const wasm::_Table& source = {}) {
			return wasm::_InstTable{ wasm::_InstTable::Type::copy, dest, source };
		}
	};

	struct Ref {
		static constexpr wasm::_InstSimple IsNull() {
			return wasm::_InstSimple{ wasm::_InstSimple::Type::refTestNull, wasm::_OpType::i32 };
		}
		static constexpr wasm::_InstSimple NullFunction() {
			return wasm::_InstSimple{ wasm::_InstSimple::Type::refNullFunction, wasm::_OpType::i32 };
		}
		static constexpr wasm::_InstSimple NullExtern() {
			return wasm::_InstSimple{ wasm::_InstSimple::Type::refNullExtern, wasm::_OpType::i32 };
		}
		static constexpr wasm::_InstFunction Function(const wasm::_Function& fn) {
			return wasm::_InstFunction{ wasm::_InstFunction::Type::refFunction, fn };
		}
	};

	struct I32 :
		public detail::_Constant<wasm::_OpType::i32, true>,
		public detail::_Compare<wasm::_OpType::i32, true>,
		public detail::_Arithmetic<wasm::_OpType::i32, true>,
		public detail::_SmallConvert<wasm::_OpType::i32>,
		public detail::_IntConvert<wasm::_OpType::i32, true>,
		public detail::_Bitwise<wasm::_OpType::i32, true>,
		public detail::_Memory<wasm::_OpType::i32>,
		public detail::_IntMemory<wasm::_OpType::i32, true>
	{};

	struct U32 :
		public detail::_Constant<wasm::_OpType::i32, false>,
		public detail::_Compare<wasm::_OpType::i32, false>,
		public detail::_Arithmetic<wasm::_OpType::i32, false>,
		public detail::_SmallConvert<wasm::_OpType::i32>,
		public detail::_IntConvert<wasm::_OpType::i32, false>,
		public detail::_Bitwise<wasm::_OpType::i32, false>,
		public detail::_Memory<wasm::_OpType::i32>,
		public detail::_IntMemory<wasm::_OpType::i32, false>
	{};

	struct I64 :
		public detail::_Constant<wasm::_OpType::i64, true>,
		public detail::_Compare<wasm::_OpType::i64, true>,
		public detail::_Arithmetic<wasm::_OpType::i64, true>,
		public detail::_LargeConvert<wasm::_OpType::i64>,
		public detail::_IntConvert<wasm::_OpType::i64, true>,
		public detail::_Bitwise<wasm::_OpType::i64, true>,
		public detail::_Memory<wasm::_OpType::i64>,
		public detail::_IntMemory<wasm::_OpType::i64, true>,
		public detail::_LargeMemory<wasm::_OpType::i64, true>
	{};

	struct U64 :
		public detail::_Constant<wasm::_OpType::i64, false>,
		public detail::_Compare<wasm::_OpType::i64, false>,
		public detail::_Arithmetic<wasm::_OpType::i64, false>,
		public detail::_LargeConvert<wasm::_OpType::i64>,
		public detail::_IntConvert<wasm::_OpType::i64, false>,
		public detail::_Bitwise<wasm::_OpType::i64, false>,
		public detail::_Memory<wasm::_OpType::i64>,
		public detail::_IntMemory<wasm::_OpType::i64, false>,
		public detail::_LargeMemory<wasm::_OpType::i64, false>
	{};

	struct F32 :
		public detail::_Constant<wasm::_OpType::f32, false>,
		public detail::_Compare<wasm::_OpType::f32, false>,
		public detail::_Arithmetic<wasm::_OpType::f32, false>,
		public detail::_SmallConvert<wasm::_OpType::f32>,
		public detail::_FloatConvert<wasm::_OpType::f32>,
		public detail::_Float<wasm::_OpType::f32>,
		public detail::_Memory<wasm::_OpType::f32>
	{};

	struct F64 :
		public detail::_Constant<wasm::_OpType::f64, false>,
		public detail::_Compare<wasm::_OpType::f64, false>,
		public detail::_Arithmetic<wasm::_OpType::f64, false>,
		public detail::_LargeConvert<wasm::_OpType::f64>,
		public detail::_FloatConvert<wasm::_OpType::f64>,
		public detail::_Float<wasm::_OpType::f64>,
		public detail::_Memory<wasm::_OpType::f64>
	{};

	static constexpr wasm::_InstSimple Drop() {
		return wasm::_InstSimple{ wasm::_InstSimple::Type::drop, wasm::_OpType::i32 };
	}
	static constexpr wasm::_InstSimple Nop() {
		return wasm::_InstSimple{ wasm::_InstSimple::Type::nop, wasm::_OpType::i32 };
	}
	static constexpr wasm::_InstSimple Return() {
		return wasm::_InstSimple{ wasm::_InstSimple::Type::ret, wasm::_OpType::i32 };
	}
	static constexpr wasm::_InstSimple Unreachable() {
		return wasm::_InstSimple{ wasm::_InstSimple::Type::unreachable, wasm::_OpType::i32 };
	}
	static constexpr wasm::_InstSimple Select() {
		return wasm::_InstSimple{ wasm::_InstSimple::Type::select, wasm::_OpType::i32 };
	}
	static constexpr wasm::_InstSimple Select(wasm::_Type type) {
		if (type == wasm::_Type::refExtern)
			return wasm::_InstSimple{ wasm::_InstSimple::Type::selectRefExtern, wasm::_OpType::i32 };
		else if (type == wasm::_Type::refFunction)
			return wasm::_InstSimple{ wasm::_InstSimple::Type::selectRefFunction, wasm::_OpType::i32 };
		return wasm::_InstSimple{ wasm::_InstSimple::Type::select, wasm::_OpType::i32 };
	}
}
