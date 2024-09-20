#pragma once

#include "wasm-instbase.h"

namespace wasm {
	/* writer must provide a type Inst, which must provide static
	*	functions for all of the separate instruction functions */
	template <class Type>
	concept IsInst = requires(uint32_t o) {
		typename Type::Inst;
		{ Type::Inst::Consti32(0lu) } -> std::same_as<typename Type::Instruction>;
		{ Type::Inst::Consti64(0llu) } -> std::same_as<typename Type::Instruction>;
		{ Type::Inst::Constf32(0.0f) } -> std::same_as<typename Type::Instruction>;
		{ Type::Inst::Constf64(0.0) } -> std::same_as<typename Type::Instruction>;
		{ Type::Inst::NoOp(wasm::NoOpType::equal, wasm::OperandType::i32) } -> std::same_as<typename Type::Instruction>;
		{ Type::Inst::Memory(wasm::MemOpType::load, std::declval<const typename Type::Memory*>(), std::declval<const typename Type::Memory*>(), o, wasm::OperandType::i32) } -> std::same_as<typename Type::Instruction>;
		{ Type::Inst::Table(wasm::TabOpType::get, std::declval<const typename Type::Table*>(), std::declval<const typename Type::Table*>()) } -> std::same_as<typename Type::Instruction>;
		{ Type::Inst::Local(wasm::LocOpType::get, std::declval<const typename Type::Variable&>()) }  -> std::same_as<typename Type::Instruction>;
		{ Type::Inst::Global(wasm::GlobOpType::get, std::declval<const typename Type::Global&>()) }  -> std::same_as<typename Type::Instruction>;
		{ Type::Inst::Ref(wasm::RefOpType::testNull, std::declval<const typename Type::Function*>()) }  -> std::same_as<typename Type::Instruction>;
		{ Type::Inst::Call(wasm::CallOpType::normal, std::declval<const typename Type::Function&>()) }  -> std::same_as<typename Type::Instruction>;
		{ Type::Inst::Indirect(wasm::CallOpType::normal, std::declval<const typename Type::Table*>(), std::declval<const typename Type::Prototype*>()) }  -> std::same_as<typename Type::Instruction>;
		{ Type::Inst::Branch(wasm::BrOpType::direct, std::declval<const typename Type::Target&>(), std::declval<const typename Type::Target*>(), o) }  -> std::same_as<typename Type::Instruction>;
	};

	/* instruction instantiation structure */
	template <wasm::IsInst Writer>
	struct Inst {
		struct Branch {
			static constexpr wasm::Instruction<Writer> Direct(const wasm::Target<Writer>& target) {
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::Branch(wasm::BrOpType::direct, target.self, 0, 0)) };
			}
			static constexpr wasm::Instruction<Writer> If(const wasm::Target<Writer>& target) {
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::Branch(wasm::BrOpType::conditional, target.self, 0, 0)) };
			}
			static constexpr wasm::Instruction<Writer> Table(std::initializer_list<const wasm::Target<Writer>&> optTarget, const wasm::Target<Writer>& defTarget) {
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::Branch(wasm::BrOpType::conditional, defTarget.self, optTarget.begin(), uint32_t(optTarget.size()))) };
			}
		};

		struct Call {
			static constexpr wasm::Instruction<Writer> Direct(const wasm::Function<Writer>& fn) {
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::Call(wasm::CallOpType::normal, fn.self)) };
			}
			static constexpr wasm::Instruction<Writer> Tail(const wasm::Function<Writer>& fn) {
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::Call(wasm::CallOpType::tail, fn.self)) };
			}
			static constexpr wasm::Instruction<Writer> Indirect() {
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::Indirect(wasm::CallOpType::normal, 0, 0)) };
			}
			static constexpr wasm::Instruction<Writer> Indirect(const wasm::Table<Writer>& table) {
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::Indirect(wasm::CallOpType::normal, &table.self, 0)) };
			}
			static constexpr wasm::Instruction<Writer> Indirect(const wasm::Prototype<Writer>& type) {
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::Indirect(wasm::CallOpType::normal, 0, &type.self)) };
			}
			static constexpr wasm::Instruction<Writer> Indirect(const wasm::Table<Writer>& table, const wasm::Prototype<Writer>& type) {
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::Indirect(wasm::CallOpType::normal, &table.self, &type.self)) };
			}
			static constexpr wasm::Instruction<Writer> IndirectTail() {
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::Indirect(wasm::CallOpType::tail, 0, 0)) };
			}
			static constexpr wasm::Instruction<Writer> IndirectTail(const wasm::Table<Writer>& table) {
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::Indirect(wasm::CallOpType::tail, &table.self, 0)) };
			}
			static constexpr wasm::Instruction<Writer> IndirectTail(const wasm::Prototype<Writer>& type) {
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::Indirect(wasm::CallOpType::tail, 0, &type.self)) };
			}
			static constexpr wasm::Instruction<Writer> IndirectTail(const wasm::Table<Writer>& table, const wasm::Prototype<Writer>& type) {
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::Indirect(wasm::CallOpType::tail, &table.self, &type.self)) };
			}
		};

		struct Local {
			static constexpr wasm::Instruction<Writer> Get(const wasm::Variable<Writer>& local) {
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::Local(wasm::LocOpType::get, local.self)) };
			}
			static constexpr wasm::Instruction<Writer> Set(const wasm::Variable<Writer>& local) {
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::Local(wasm::LocOpType::set, local.self)) };
			}
			static constexpr wasm::Instruction<Writer> Tee(const wasm::Variable<Writer>& local) {
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::Local(wasm::LocOpType::tee, local.self)) };
			}
		};

		struct Global {
			static constexpr wasm::Instruction<Writer> Get(const wasm::Global<Writer>& global) {
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::Global(wasm::GlobOpType::get, global)) };
			}
			static constexpr wasm::Instruction<Writer> Set(const wasm::Global<Writer>& global) {
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::Global(wasm::GlobOpType::set, global)) };
			}
		};

		struct Memory {
			static constexpr wasm::Instruction<Writer> Grow() {
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::Memory(wasm::MemOpType::grow, 0, 0, 0, wasm::OperandType::i32)) };
			}
			static constexpr wasm::Instruction<Writer> Size() {
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::Memory(wasm::MemOpType::size, 0, 0, 0, wasm::OperandType::i32)) };
			}
			static constexpr wasm::Instruction<Writer> Fill() {
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::Memory(wasm::MemOpType::fill, 0, 0, 0, wasm::OperandType::i32)) };
			}
			static constexpr wasm::Instruction<Writer> Copy() {
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::Memory(wasm::MemOpType::copy, 0, 0, wasm::OperandType::i32)) };
			}
			static constexpr wasm::Instruction<Writer> Grow(const wasm::Memory<Writer>& memory) {
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::Memory(wasm::MemOpType::grow, &memory.self, 0, 0, wasm::OperandType::i32)) };
			}
			static constexpr wasm::Instruction<Writer> Size(const wasm::Memory<Writer>& memory) {
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::Memory(wasm::MemOpType::size, &memory.self, 0, 0, wasm::OperandType::i32)) };
			}
			static constexpr wasm::Instruction<Writer> Fill(const wasm::Memory<Writer>& memory) {
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::Memory(wasm::MemOpType::fill, &memory.self, 0, 0, wasm::OperandType::i32)) };
			}
			static constexpr wasm::Instruction<Writer> Copy(const wasm::Memory<Writer>& memory) {
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::Memory(wasm::MemOpType::copy, &memory.self, &memory.self, 0, wasm::OperandType::i32)) };
			}
			static constexpr wasm::Instruction<Writer> Copy(const wasm::Memory<Writer>* dest, const wasm::Memory<Writer>* source) {
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::Memory(wasm::MemOpType::copy, (dest == 0 ? 0 : &dest->self), (source == 0 ? 0 : &source->self), 0, wasm::OperandType::i32)) };
			}
		};

		struct Table {
			static constexpr wasm::Instruction<Writer> Get() {
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::Table(wasm::TabOpType::get, 0, 0)) };
			}
			static constexpr wasm::Instruction<Writer> Set() {
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::Table(wasm::TabOpType::set, 0, 0)) };
			}
			static constexpr wasm::Instruction<Writer> Size() {
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::Table(wasm::TabOpType::size, 0, 0)) };
			}
			static constexpr wasm::Instruction<Writer> Grow() {
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::Table(wasm::TabOpType::grow, 0, 0)) };
			}
			static constexpr wasm::Instruction<Writer> Fill() {
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::Table(wasm::TabOpType::fill, 0, 0)) };
			}
			static constexpr wasm::Instruction<Writer> Copy() {
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::Table(wasm::TabOpType::copy, 0, 0)) };
			}
			static constexpr wasm::Instruction<Writer> Get(const wasm::Table<Writer>& table) {
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::Table(wasm::TabOpType::get, &table.self, 0)) };
			}
			static constexpr wasm::Instruction<Writer> Set(const wasm::Table<Writer>& table) {
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::Table(wasm::TabOpType::set, &table.self, 0)) };
			}
			static constexpr wasm::Instruction<Writer> Size(const wasm::Table<Writer>& table) {
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::Table(wasm::TabOpType::size, &table.self, 0)) };
			}
			static constexpr wasm::Instruction<Writer> Grow(const wasm::Table<Writer>& table) {
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::Table(wasm::TabOpType::grow, &table.self, 0)) };
			}
			static constexpr wasm::Instruction<Writer> Fill(const wasm::Table<Writer>& table) {
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::Table(wasm::TabOpType::fill, &table.self, 0)) };
			}
			static constexpr wasm::Instruction<Writer> Copy(const wasm::Table<Writer>& table) {
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::Table(wasm::TabOpType::copy, &table.self, 0)) };
			}
			static constexpr wasm::Instruction<Writer> Copy(const wasm::Table<Writer>* dest, const wasm::Table<Writer>* source) {
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::Table(wasm::TabOpType::copy, (dest == 0 ? 0 : &dest->self), (source == 0 ? 0 : &source->self))) };
			}
		};

		struct Ref {
			static constexpr wasm::Instruction<Writer> IsNull() {
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::Ref(wasm::RefOpType::testNull, 0)) };
			}
			static constexpr wasm::Instruction<Writer> NullFunction() {
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::Ref(wasm::RefOpType::nullFunction, 0)) };
			}
			static constexpr wasm::Instruction<Writer> NullExtern() {
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::Ref(wasm::RefOpType::nullExtern, 0)) };
			}
			static constexpr wasm::Instruction<Writer> Function(const wasm::Function<Writer>& fn) {
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::Ref(wasm::RefOpType::function, &fn.self)) };
			}
		};

		struct I32 :
			public detail::Constant<Writer, wasm::OperandType::i32, true>,
			public detail::Compare<Writer, wasm::OperandType::i32, true>,
			public detail::Arithmetic<Writer, wasm::OperandType::i32, true>,
			public detail::SmallConvert<Writer, wasm::OperandType::i32>,
			public detail::IntConvert<Writer, wasm::OperandType::i32, true>,
			public detail::Bitwise<Writer, wasm::OperandType::i32, true>,
			public detail::Memory<Writer, wasm::OperandType::i32>,
			public detail::IntMemory<Writer, wasm::OperandType::i32, true>
		{};

		struct U32 :
			public detail::Constant<Writer, wasm::OperandType::i32, false>,
			public detail::Compare<Writer, wasm::OperandType::i32, false>,
			public detail::Arithmetic<Writer, wasm::OperandType::i32, false>,
			public detail::SmallConvert<Writer, wasm::OperandType::i32>,
			public detail::IntConvert<Writer, wasm::OperandType::i32, false>,
			public detail::Bitwise<Writer, wasm::OperandType::i32, false>,
			public detail::Memory<Writer, wasm::OperandType::i32>,
			public detail::IntMemory<Writer, wasm::OperandType::i32, false>
		{};

		struct I64 :
			public detail::Constant<Writer, wasm::OperandType::i64, true>,
			public detail::Compare<Writer, wasm::OperandType::i64, true>,
			public detail::Arithmetic<Writer, wasm::OperandType::i64, true>,
			public detail::LargeConvert<Writer, wasm::OperandType::i64>,
			public detail::IntConvert<Writer, wasm::OperandType::i64, true>,
			public detail::Bitwise<Writer, wasm::OperandType::i64, true>,
			public detail::Memory<Writer, wasm::OperandType::i64>,
			public detail::IntMemory<Writer, wasm::OperandType::i64, true>,
			public detail::LargeMemory<Writer, wasm::OperandType::i64, true>
		{};

		struct U64 :
			public detail::Constant<Writer, wasm::OperandType::i64, false>,
			public detail::Compare<Writer, wasm::OperandType::i64, false>,
			public detail::Arithmetic<Writer, wasm::OperandType::i64, false>,
			public detail::LargeConvert<Writer, wasm::OperandType::i64>,
			public detail::IntConvert<Writer, wasm::OperandType::i64, false>,
			public detail::Bitwise<Writer, wasm::OperandType::i64, false>,
			public detail::Memory<Writer, wasm::OperandType::i64>,
			public detail::IntMemory<Writer, wasm::OperandType::i64, false>,
			public detail::LargeMemory<Writer, wasm::OperandType::i64, false>
		{};

		struct F32 :
			public detail::Constant<Writer, wasm::OperandType::f32, false>,
			public detail::Compare<Writer, wasm::OperandType::f32, false>,
			public detail::Arithmetic<Writer, wasm::OperandType::f32, false>,
			public detail::SmallConvert<Writer, wasm::OperandType::f32>,
			public detail::FloatConvert<Writer, wasm::OperandType::f32>,
			public detail::Float<Writer, wasm::OperandType::f32>,
			public detail::Memory<Writer, wasm::OperandType::f32>
		{};

		struct F64 :
			public detail::Constant<Writer, wasm::OperandType::f64, false>,
			public detail::Compare<Writer, wasm::OperandType::f64, false>,
			public detail::Arithmetic<Writer, wasm::OperandType::f64, false>,
			public detail::LargeConvert<Writer, wasm::OperandType::f64>,
			public detail::FloatConvert<Writer, wasm::OperandType::f64>,
			public detail::Float<Writer, wasm::OperandType::f64>,
			public detail::Memory<Writer, wasm::OperandType::f64>
		{};

		static constexpr wasm::Instruction<Writer> Drop() {
			return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::drop, wasm::OperandType::i32)) };
		}
		static constexpr wasm::Instruction<Writer> Nop() {
			return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::nop, wasm::OperandType::i32)) };
		}
		static constexpr wasm::Instruction<Writer> Return() {
			return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::ret, wasm::OperandType::i32)) };
		}
		static constexpr wasm::Instruction<Writer> Unreachable() {
			return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::unreachable, wasm::OperandType::i32)) };
		}
		static constexpr wasm::Instruction<Writer> Select() {
			return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::select, wasm::OperandType::i32)) };
		}
		static constexpr wasm::Instruction<Writer> Select(wasm::Type type) {
			if (type == wasm::Type::refExtern)
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::selectRefExtern, wasm::OperandType::i32)) };
			else if (type == wasm::Type::refFunction)
				return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::selectRefFunction, wasm::OperandType::i32)) };
			return wasm::Instruction<Writer>{ std::move(Writer::Inst::NoOp(wasm::NoOpType::select, wasm::OperandType::i32)) };
		}
	};
}
