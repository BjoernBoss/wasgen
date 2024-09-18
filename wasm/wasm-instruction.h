#pragma once

#include "wasm-instbase.h"

namespace inst {
	template <wasm::IsInst Writer>
	struct Local {
		static constexpr wasm::Instruction<Writer> Get(const wasm::Local<Writer>& local) {
			return wasm::Instruction<Writer>{ std::move(Writer::Inst::Local(local, wasm::VarOpType::get)) };
		}
		static constexpr wasm::Instruction<Writer> Set(const wasm::Local<Writer>& local) {
			return wasm::Instruction<Writer>{ std::move(Writer::Inst::Local(local, wasm::VarOpType::set)) };
		}
		static constexpr wasm::Instruction<Writer> Tee(const wasm::Local<Writer>& local) {
			return wasm::Instruction<Writer>{ std::move(Writer::Inst::Local(local, wasm::VarOpType::tee)) };
		}
	};

	template <wasm::IsInst Writer>
	struct Global {
		static constexpr wasm::Instruction<Writer> Get(const wasm::Global<Writer>& global) {
			return wasm::Instruction<Writer>{ std::move(Writer::Inst::Global(global, wasm::VarOpType::get)) };
		}
		static constexpr wasm::Instruction<Writer> Set(const wasm::Global<Writer>& global) {
			return wasm::Instruction<Writer>{ std::move(Writer::Inst::Global(global, wasm::VarOpType::set)) };
		}
	};

	template <wasm::IsInst Writer>
	struct Memory {
		static constexpr wasm::Instruction<Writer> Grow(size_t pages, const wasm::Memory<Writer>* memory = 0) {
			return wasm::Instruction<Writer>{ std::move(Writer::Inst::Memory(0, 0, pages, memory, 0, wasm::MemOpType::grow)) };
		}
		static constexpr wasm::Instruction<Writer> Size(const wasm::Memory<Writer>* memory = 0) {
			return wasm::Instruction<Writer>{ std::move(Writer::Inst::Memory(0, 0, 0, 0, 0, wasm::MemOpType::size)) };
		}
		static constexpr wasm::Instruction<Writer> Copy(size_t dstAddress, size_t srcAddress, size_t count, const wasm::Memory<Writer>* dstMemory = 0, const wasm::Memory<Writer>* srcMemory = 0) {
			return wasm::Instruction<Writer>{ std::move(Writer::Inst::Memory(dstAddress, srcAddress, count, dstMemory, srcMemory, wasm::MemOpType::copy)) };
		}
		static constexpr wasm::Instruction<Writer> Fill(size_t address, uint8_t value, size_t count, const wasm::Memory<Writer>* memory = 0) {
			return wasm::Instruction<Writer>{ std::move(Writer::Inst::Memory(address, value, count, memory, 0, wasm::MemOpType::fill)) };
		}
	};

	template <wasm::IsInst Writer>
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

	template <wasm::IsInst Writer>
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

	template <wasm::IsInst Writer>
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

	template <wasm::IsInst Writer>
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

	template <wasm::IsInst Writer>
	struct F32 :
		public detail::Constant<Writer, wasm::OperandType::f32, false>,
		public detail::Compare<Writer, wasm::OperandType::f32, false>,
		public detail::Arithmetic<Writer, wasm::OperandType::f32, false>,
		public detail::SmallConvert<Writer, wasm::OperandType::f32>,
		public detail::FloatConvert<Writer, wasm::OperandType::f32>,
		public detail::Float<Writer, wasm::OperandType::f32>,
		public detail::Memory<Writer, wasm::OperandType::f32>
	{};

	template <wasm::IsInst Writer>
	struct F64 :
		public detail::Constant<Writer, wasm::OperandType::f64, false>,
		public detail::Compare<Writer, wasm::OperandType::f64, false>,
		public detail::Arithmetic<Writer, wasm::OperandType::f64, false>,
		public detail::LargeConvert<Writer, wasm::OperandType::f64>,
		public detail::FloatConvert<Writer, wasm::OperandType::f64>,
		public detail::Float<Writer, wasm::OperandType::f64>,
		public detail::Memory<Writer, wasm::OperandType::f64>
	{};
}
