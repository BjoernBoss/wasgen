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
		if (exchange.name.empty())
			return std::u8string();
		if (exchange.module.empty())
			return str::Format<std::u8string>(u8" (export \"{}\")", exchange.name);
		return str::Format<std::u8string>(u8" (import \"{}\" \"{}\")", exchange.module, exchange.name);
	}
	static std::u8string fMakePrototype(const wasm::Proto& prototype) {
		std::u8string out;
		for (size_t i = 0; i < prototype.params.size(); ++i)
			str::FormatTo(out, " (param{} {})", fMakeId(prototype.params[i].id), fMakeType(prototype.params[i].type));
		if (!prototype.result.empty()) {
			out += u8" (result";
			for (size_t i = 0; i < prototype.result.size(); ++i)
				out.append(1, u8' ').append(fMakeType(prototype.result[i]));
			out += u8")";
		}
		return out;
	}

public:
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

	/* wasm::IsInstBase */
	struct InstBase {
		std::u8string value;
		constexpr InstBase(std::u8string value) : value{ value } {}
	};

	/* wasm::IsJumpBase */
	struct JumpBase {
		std::u8string id;
		constexpr JumpBase(std::u8string id) : id{ id } {}
	};

	/* wasm::IsLocal */
	struct Local {
		std::u8string id;
		constexpr Local(std::u8string id) : id{ id } {}
	};

	/* wasm::IsFunction */
	struct Function {
		std::u8string id;
		constexpr Function(std::u8string id) : id{ id } {}

	public:
		constexpr void popTarget() {
		}
		constexpr void popThen() {
		}
		constexpr void popElse() {
		}
		constexpr void toggleElse() {
		}
		constexpr TextWriter::Local addLocal(wasm::Type type, std::u8string_view id) {
			return TextWriter::Local{ std::u8string(id) };
		}
		constexpr void addInst(const TextWriter::InstBase& inst) {

		}
		constexpr TextWriter::JumpBase pushJumpBase(const std::u8string_view& label, const wasm::Proto& prototype, bool loop) {
			return TextWriter::JumpBase{ std::u8string(label) };
		}
		constexpr void pushConditional(const wasm::Proto& prototype) {
		}
	};

	/* wasm::IsModule */
	struct Module {
		std::u8string out;

	public:
		constexpr Module(const TextWriter& state) {}

	public:
		constexpr TextWriter::Memory addMemory(const std::u8string_view& id, const wasm::Limit& limit, const wasm::Exchange& exchange) {
			str::FormatTo(out, u8"\n  (memory{}{}{})", fMakeId(id), fMakeExchange(exchange), fMakeLimit(limit));
			return TextWriter::Memory{ std::u8string(id) };
		}
		constexpr TextWriter::Table addTable(const std::u8string_view& id, bool functions, const wasm::Limit& limit, const wasm::Exchange& exchange) {
			str::FormatTo(out, u8"\n  (table{}{}{} {})", fMakeId(id), fMakeExchange(exchange), fMakeLimit(limit), functions ? u8"funcref" : u8"externref");
			return TextWriter::Table{ std::u8string(id) };
		}
		constexpr TextWriter::Function addFunction(const std::u8string_view& id, const wasm::Proto& prototype, const wasm::Exchange& exchange) {
			str::BuildTo(out, u8"\n  (func", fMakeId(id), fMakeExchange(exchange), fMakePrototype(prototype), u8')');
			return TextWriter::Function{ std::u8string(id) };
		}
		constexpr std::u8string toString() const {
			if (out.empty())
				return u8"(module)";
			return str::Build<std::u8string>(u8"(module", out, u8"\n)");
		}
	};

	/* instructions: no-operands */
	static constexpr TextWriter::InstBase NoOpInst(wasm::NoOpInst type) {
		switch (type) {
		case wasm::NoOpInst::add:
			return InstBase{ u8"add" };
		case wasm::NoOpInst::ret:
			return InstBase{ u8"ret" };
		default:
			break;
		}
		env::fail(str::Format<std::u8string>(u8"Unknown wasm::NoOpInst {}\n", size_t(type)));
		return InstBase{ u8"" };
	}
	static constexpr TextWriter::InstBase InstConst(uint32_t v) {
		return InstBase{ str::Build<std::u8string>(u8"i32.const ", v) };
	}
};
