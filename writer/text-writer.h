#pragma once

#include "text-sink.h"
#include "text-module.h"

namespace writer {
	struct TextWriter {
		/* wasm::IsVariable */
		using Variable = writer::text::Variable;

		/* wasm::IsInstruction */
		using Instruction = writer::text::Instruction;

		/* wasm::IsTarget */
		using Target = writer::text::Target;

		/* wasm::IsSink */
		using Sink = writer::text::SinkWrapper;

		/* wasm::IsModule */
		using Module = writer::text::Module;

		/* wasm::IsPrototype */
		using Prototype = writer::text::Prototype;

		/* wasm::IsMemory */
		using Memory = writer::text::Memory;

		/* wasm::IsTable */
		using Table = writer::text::Table;

		/* wasm::IsFunction */
		using Function = writer::text::Function;

		/* wasm::IsGlobal */
		using Global = writer::text::Global;

		/* wasm::IsInst */
		struct Inst {
			static constexpr TextWriter::Instruction Consti32(uint32_t v) {
				return TextWriter::Instruction{ str::Build<std::u8string>(u8"i32.const ", v) };
			}
			static constexpr TextWriter::Instruction Consti64(uint64_t v) {
				return TextWriter::Instruction{ str::Build<std::u8string>(u8"i64.const ", v) };
			}
			static constexpr TextWriter::Instruction Constf32(float v) {
				return TextWriter::Instruction{ str::Build<std::u8string>(u8"f32.const ", v) };
			}
			static constexpr TextWriter::Instruction Constf64(double v) {
				return TextWriter::Instruction{ str::Build<std::u8string>(u8"f64.const ", v) };
			}
			static constexpr TextWriter::Instruction NoOp(wasm::NoOpType op, wasm::OperandType type) {
				return TextWriter::Instruction{ u8"%not-implemented%" };
			}
			static constexpr TextWriter::Instruction Memory(wasm::MemOpType op, const TextWriter::Memory* mem, const TextWriter::Memory* src, uint32_t off, wasm::OperandType type) {
				return TextWriter::Instruction{ u8"%not-implemented%" };
			}
			static constexpr TextWriter::Instruction Table(wasm::TabOpType op, const TextWriter::Table* tab, const TextWriter::Table* src) {
				return TextWriter::Instruction{ u8"%not-implemented%" };
			}
			static constexpr TextWriter::Instruction Local(wasm::LocOpType op, const TextWriter::Variable& loc) {
				return TextWriter::Instruction{ u8"%not-implemented%" };
			}
			static constexpr TextWriter::Instruction Global(wasm::GlobOpType op, const TextWriter::Global& glob) {
				return TextWriter::Instruction{ u8"%not-implemented%" };
			}
			static constexpr TextWriter::Instruction Ref(wasm::RefOpType op, const TextWriter::Function* fn) {
				return TextWriter::Instruction{ u8"%not-implemented%" };
			}
			static constexpr TextWriter::Instruction Call(wasm::CallOpType op, const TextWriter::Function& fn) {
				return TextWriter::Instruction{ u8"%not-implemented%" };
			}
			static constexpr TextWriter::Instruction Indirect(wasm::CallOpType op, const TextWriter::Table* tab, const TextWriter::Prototype* type) {
				return TextWriter::Instruction{ u8"%not-implemented%" };
			}
			static constexpr TextWriter::Instruction Branch(wasm::BrOpType op, const TextWriter::Target& tgt, const TextWriter::Target* list, uint32_t count) {
				return TextWriter::Instruction{ u8"%not-implemented%" };
			}
		};
	};
}
