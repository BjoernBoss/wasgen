/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2024-2026 Bjoern Boss Henrichsen */
#pragma once

#include "wasm-instbase.h"

namespace wasm::inst {
	struct Branch {
		static constexpr wasm::InstBranch Direct(const wasm::Target& target) {
			return wasm::InstBranch{ wasm::InstBranch::Type::direct, {}, target };
		}

		/* expected on stack: [condition] */
		static constexpr wasm::InstBranch If(const wasm::Target& target) {
			return wasm::InstBranch{ wasm::InstBranch::Type::conditional, {}, target };
		}

		/* expected on stack: [index] */
		static constexpr wasm::InstBranch Table(std::vector<wasm::WTarget> optTarget, const wasm::Target& defTarget) {
			return wasm::InstBranch{ wasm::InstBranch::Type::table, optTarget, defTarget };
		}
	};

	struct Call {
		/* expected on stack: [parameter] */
		static constexpr wasm::InstFunction Direct(const wasm::Function& fn) {
			return wasm::InstFunction{ wasm::InstFunction::Type::callNormal, fn };
		}

		/* expected on stack: [parameter] */
		static constexpr wasm::InstFunction Tail(const wasm::Function& fn) {
			return wasm::InstFunction{ wasm::InstFunction::Type::callTail, fn };
		}

		/* expected on stack: [parameter] [table-index] */
		static constexpr wasm::InstIndirect Indirect(const wasm::Table& table, const wasm::Prototype& type) {
			return wasm::InstIndirect{ wasm::InstIndirect::Type::callNormal, table, type };
		}

		/* expected on stack: [parameter] [table-index] */
		static constexpr wasm::InstIndirect IndirectTail(const wasm::Table& table, const wasm::Prototype& type) {
			return wasm::InstIndirect{ wasm::InstIndirect::Type::callTail, table, type };
		}

		/* expected on stack: [parameter] [table-index] */
		static constexpr wasm::InstIndirect Indirect(const wasm::Table& table, std::vector<wasm::Type> params = {}, std::vector<wasm::Type> result = {}) {
			wasm::Prototype type{};
			if (table.valid())
				type = table.module().prototype(params, result);
			return wasm::InstIndirect{ wasm::InstIndirect::Type::callNormal, table, type };
		}

		/* expected on stack: [parameter] [table-index] */
		static constexpr wasm::InstIndirect IndirectTail(const wasm::Table& table, std::vector<wasm::Type> params = {}, std::vector<wasm::Type> result = {}) {
			wasm::Prototype type{};
			if (table.valid())
				type = table.module().prototype(params, result);
			return wasm::InstIndirect{ wasm::InstIndirect::Type::callTail, table, type };
		}
	};

	struct Local {
		static constexpr wasm::InstLocal Get(const wasm::Variable& local) {
			return wasm::InstLocal{ wasm::InstLocal::Type::get, local };
		}

		/* expected on stack: [value] */
		static constexpr wasm::InstLocal Set(const wasm::Variable& local) {
			return wasm::InstLocal{ wasm::InstLocal::Type::set, local };
		}

		/* expected on stack: [value] */
		static constexpr wasm::InstLocal Tee(const wasm::Variable& local) {
			return wasm::InstLocal{ wasm::InstLocal::Type::tee, local };
		}
	};

	struct Param {
		static constexpr wasm::InstParam Get(uint32_t index) {
			return wasm::InstParam{ wasm::InstParam::Type::get, index };
		}

		/* expected on stack: [value] */
		static constexpr wasm::InstParam Set(uint32_t index) {
			return wasm::InstParam{ wasm::InstParam::Type::set, index };
		}

		/* expected on stack: [value] */
		static constexpr wasm::InstParam Tee(uint32_t index) {
			return wasm::InstParam{ wasm::InstParam::Type::tee, index };
		}
	};

	struct Global {
		static constexpr wasm::InstGlobal Get(const wasm::Global& global) {
			return wasm::InstGlobal{ wasm::InstGlobal::Type::get, global };
		}

		/* expected on stack: [value] */
		static constexpr wasm::InstGlobal Set(const wasm::Global& global) {
			return wasm::InstGlobal{ wasm::InstGlobal::Type::set, global };
		}
	};

	struct Memory {
		static constexpr wasm::InstMemory Size(const wasm::Memory& memory) {
			return wasm::InstMemory{ wasm::InstMemory::Type::size, memory, {}, 0, wasm::OpType::i32 };
		}

		/* expected on stack: [pages-to-grow-by] */
		static constexpr wasm::InstMemory Grow(const wasm::Memory& memory) {
			return wasm::InstMemory{ wasm::InstMemory::Type::grow, memory, {}, 0, wasm::OpType::i32 };
		}

		/* expected on stack: [dest-address] [value] [size] */
		static constexpr wasm::InstMemory Fill(const wasm::Memory& memory) {
			return wasm::InstMemory{ wasm::InstMemory::Type::fill, memory, {}, 0, wasm::OpType::i32 };
		}

		/* expected on stack: [dest-address] [source-address] [size] */
		static constexpr wasm::InstMemory Copy(const wasm::Memory& memory) {
			return wasm::InstMemory{ wasm::InstMemory::Type::copy, memory, memory, 0, wasm::OpType::i32 };
		}

		/* expected on stack: [dest-address] [source-address] [size] */
		static constexpr wasm::InstMemory Copy(const wasm::Memory& dest, const wasm::Memory& source) {
			return wasm::InstMemory{ wasm::InstMemory::Type::copy, source, dest, 0, wasm::OpType::i32 };
		}
	};

	struct Table {
		static constexpr wasm::InstTable Size(const wasm::Table& table) {
			return wasm::InstTable{ wasm::InstTable::Type::size, table, {} };
		}

		/* expected on stack: [index] */
		static constexpr wasm::InstTable Get(const wasm::Table& table) {
			return wasm::InstTable{ wasm::InstTable::Type::get, table, {} };
		}

		/* expected on stack: [index] [value] */
		static constexpr wasm::InstTable Set(const wasm::Table& table) {
			return wasm::InstTable{ wasm::InstTable::Type::set, table, {} };
		}

		/* expected on stack: [slots-to-grow-by] */
		static constexpr wasm::InstTable Grow(const wasm::Table& table) {
			return wasm::InstTable{ wasm::InstTable::Type::grow, table, {} };
		}

		/* expected on stack: [dest-offset] [value] [size] */
		static constexpr wasm::InstTable Fill(const wasm::Table& table) {
			return wasm::InstTable{ wasm::InstTable::Type::fill, table, {} };
		}

		/* expected on stack: [dest-offset] [source-offset] [size] */
		static constexpr wasm::InstTable Copy(const wasm::Table& table) {
			return wasm::InstTable{ wasm::InstTable::Type::copy, table, table };
		}

		/* expected on stack: [dest-offset] [source-offset] [size] */
		static constexpr wasm::InstTable Copy(const wasm::Table& dest, const wasm::Table& source) {
			return wasm::InstTable{ wasm::InstTable::Type::copy, source, dest };
		}
	};

	struct Ref {
		static constexpr wasm::InstSimple NullFunction() {
			return wasm::InstSimple{ wasm::InstSimple::Type::refNullFunction };
		}
		static constexpr wasm::InstSimple NullExtern() {
			return wasm::InstSimple{ wasm::InstSimple::Type::refNullExtern };
		}
		static constexpr wasm::InstFunction Function(const wasm::Function& fn) {
			return wasm::InstFunction{ wasm::InstFunction::Type::refFunction, fn };
		}

		/* expected on stack: [ref-object] */
		static constexpr wasm::InstSimple IsNull() {
			return wasm::InstSimple{ wasm::InstSimple::Type::refTestNull };
		}
	};

	struct I32 :
		public detail::Common<wasm::OpType::i32, true>,
		public detail::SmallConvert<false, true>,
		public detail::IntOperations<true, true>,
		public detail::Memory<wasm::OpType::i32>,
		public detail::IntMemory<wasm::OpType::i32, true>
	{};

	struct U32 :
		public detail::Common<wasm::OpType::i32, false>,
		public detail::SmallConvert<false, false>,
		public detail::IntOperations<true, false>,
		public detail::Memory<wasm::OpType::i32>,
		public detail::IntMemory<wasm::OpType::i32, false>
	{};

	struct I64 :
		public detail::Common<wasm::OpType::i64, true>,
		public detail::LargeConvert<false>,
		public detail::IntOperations<false, true>,
		public detail::Memory<wasm::OpType::i64>,
		public detail::IntMemory<wasm::OpType::i64, true>,
		public detail::LargeMemory<wasm::OpType::i64, true>
	{};

	struct U64 :
		public detail::Common<wasm::OpType::i64, false>,
		public detail::LargeConvert<false>,
		public detail::IntOperations<false, false>,
		public detail::Memory<wasm::OpType::i64>,
		public detail::IntMemory<wasm::OpType::i64, false>,
		public detail::LargeMemory<wasm::OpType::i64, false>
	{};

	struct F32 :
		public detail::Common<wasm::OpType::f32, false>,
		public detail::SmallConvert<true, true>,
		public detail::FloatOperations<true>,
		public detail::Memory<wasm::OpType::f32>
	{};

	struct F64 :
		public detail::Common<wasm::OpType::f64, false>,
		public detail::LargeConvert<true>,
		public detail::FloatOperations<false>,
		public detail::Memory<wasm::OpType::f64>
	{};

	static constexpr wasm::InstSimple Drop() {
		return wasm::InstSimple{ wasm::InstSimple::Type::drop };
	}
	static constexpr wasm::InstSimple Nop() {
		return wasm::InstSimple{ wasm::InstSimple::Type::nop };
	}
	static constexpr wasm::InstSimple Return() {
		return wasm::InstSimple{ wasm::InstSimple::Type::ret };
	}
	static constexpr wasm::InstSimple Unreachable() {
		return wasm::InstSimple{ wasm::InstSimple::Type::unreachable };
	}

	/* expected on stack: [true-value] [false-value] [condition] */
	static constexpr wasm::InstSimple Select() {
		return wasm::InstSimple{ wasm::InstSimple::Type::select };
	}

	/* expected on stack: [true-value] [false-value] [condition] */
	static constexpr wasm::InstSimple Select(wasm::Type type) {
		if (type == wasm::Type::refExtern)
			return wasm::InstSimple{ wasm::InstSimple::Type::selectRefExtern };
		else if (type == wasm::Type::refFunction)
			return wasm::InstSimple{ wasm::InstSimple::Type::selectRefFunction };
		return wasm::InstSimple{ wasm::InstSimple::Type::select };
	}
}
