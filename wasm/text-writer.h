#pragma once

#include "wasm-module.h"
#include "../interface/interface.h"

#include <ustring/str.h>
#include <algorithm>

struct TextWriter {
private:
	static std::u8string_view fMakeType(wasm::Type type) {
		switch (type) {
		case wasm::Type::i64:
			return u8"i64";
		case wasm::Type::f32:
			return u8"f32";
		case wasm::Type::f64:
			return u8"f64";
		case wasm::Type::refExtern:
			return u8"externref";
		case wasm::Type::refFunction:
			return u8"funcref";
		case wasm::Type::v128:
			return u8"v128";
		case wasm::Type::i32:
		default:
			return u8"i32";
		}
	}
	static std::u8string fMakeId(const std::u8string_view& id) {
		if (id.empty())
			return std::u8string();
		return str::Build<std::u8string>(U" $", id);
	}
	static std::u8string fMakeLimit(const wasm::Limit& limit) {
		if (limit.max == std::numeric_limits<uint32_t>::max())
			return str::Build<std::u8string>(u8' ', limit.min);
		return str::Build<std::u8string>(u8' ', limit.min, u8' ', limit.max);
	}
	static std::u8string fMakeExchange(const wasm::Exchange& exchange) {
		std::u8string out;
		if (!exchange.expName.empty())
			str::FormatTo(out, u8" (export \"{}\")", exchange.expName);
		if (!exchange.impName.empty())
			str::FormatTo(out, u8" (import \"{}\" \"{}\")", exchange.impModule, exchange.impName);
		return out;
	}
	//static std::u8string fMakePrototype(const wasm::Proto& prototype) {
	//	std::u8string out;
	//	for (size_t i = 0; i < prototype.params.size(); ++i)
	//		str::FormatTo(out, " (param{} {})", fMakeId(prototype.params[i].id), fMakeType(prototype.params[i].type));
	//	if (!prototype.result.empty()) {
	//		out += u8" (result";
	//		for (size_t i = 0; i < prototype.result.size(); ++i)
	//			out.append(1, u8' ').append(fMakeType(prototype.result[i]));
	//		out += u8")";
	//	}
	//	return out;
	//}

public:
	/* wasm::IsPrototype */
	struct Prototype {
		std::u8string id;
		constexpr Prototype(std::u8string id) : id{ id } {}
	};

	/* wasm::IsMemory */
	struct Memory {
		std::u8string id;
		constexpr Memory(std::u8string id) : id{ id } {}
	};

	/* wasm::IsTable */
	struct Table {
		std::u8string id;
		constexpr Table(std::u8string id) : id{ id } {}
	};

	/* wasm::IsVariable */
	struct Variable {
		std::u8string id;
		constexpr Variable(std::u8string id) : id{ id } {}
	};

	/* wasm::IsGlobal */
	struct Global {
		std::u8string id;
		constexpr Global(std::u8string id) : id{ id } {}
	};

	/* wasm::IsFunction */
	struct Function {
		std::u8string id;
		constexpr Function(std::u8string id) : id{ id } {}
	};

	/* wasm::IsInstruction */
	struct Instruction {
		std::u8string value;
		constexpr Instruction(std::u8string value) : value{ value } {}
	};

	/* wasm::IsTarget */
	struct Target {
		std::u8string id;
		constexpr Target(std::u8string id) : id{ id } {}
	};

	/* wasm::IsSink */
	struct Sink {
		constexpr Sink() {}

	public:
		constexpr void popTarget() {
		}
		constexpr void popThen() {
		}
		constexpr void popElse() {
		}
		constexpr void toggleElse() {
		}
		constexpr TextWriter::Variable getParameter(uint32_t index) {
			return TextWriter::Variable{ std::u8string() };
		}
		constexpr TextWriter::Variable addLocal(wasm::Type type, std::u8string_view id) {
			return TextWriter::Variable{ std::u8string(id) };
		}
		constexpr void addInst(const TextWriter::Instruction& inst) {

		}
		constexpr TextWriter::Target pushTarget(const std::u8string_view& label, const TextWriter::Prototype* prototype, bool loop) {
			return TextWriter::Target{ std::u8string(label) };
		}
		constexpr void pushConditional(const TextWriter::Prototype* prototype) {
		}
	};

	/* wasm::IsModule */
	struct Module {
		std::u8string out;

	public:
		constexpr Module(const TextWriter& state) {}

	public:
		constexpr TextWriter::Prototype addPrototype(const std::u8string_view& id, const wasm::Param* params, size_t paramCount, const wasm::Type* results, size_t resultCount) {
			return TextWriter::Prototype{ std::u8string(id) };
		}
		constexpr TextWriter::Memory addMemory(const std::u8string_view& id, const wasm::Limit& limit, const wasm::Exchange& exchange) {
			str::BuildTo(out, u8"\n  (memory", fMakeId(id), fMakeExchange(exchange), fMakeLimit(limit), u8')');
			return TextWriter::Memory{ std::u8string(id) };
		}
		constexpr TextWriter::Table addTable(const std::u8string_view& id, bool functions, const wasm::Limit& limit, const wasm::Exchange& exchange) {
			str::BuildTo(out, u8"\n  (table", fMakeId(id), fMakeExchange(exchange), fMakeLimit(limit), functions ? u8" funcref)" : u8" externref)");
			return TextWriter::Table{ std::u8string(id) };
		}
		constexpr TextWriter::Global addGlobal(const std::u8string_view& id, wasm::Type type, const wasm::Exchange& exchange) {
			return TextWriter::Global{ std::u8string(id) };
		}
		constexpr TextWriter::Function addFunction(const std::u8string_view& id, const TextWriter::Prototype* prototype, const wasm::Exchange& exchange) {
			// str::BuildTo(out, u8"\n  (func", fMakeId(id), fMakeExchange(exchange), fMakePrototype(prototype), u8')');
			return TextWriter::Function{ std::u8string(id) };
		}
		constexpr TextWriter::Sink getSink(const TextWriter::Function& fn) {
			return TextWriter::Sink{};
		}
		constexpr std::u8string toString() const {
			if (out.empty())
				return u8"(module)";
			return str::Build<std::u8string>(u8"(module", out, u8"\n)");
		}
	};

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
