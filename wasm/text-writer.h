#pragma once

#include "wasm.h"
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
	static std::u8string fMakePrototype(const wasm::Prototype& prototype) {
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
	/* wasm::CanMemory */
	struct Memory {
		std::u8string id;
		constexpr Memory() = default;
		constexpr Memory(std::u8string id) : id{ id } {}
	};

	/* wasm::CanTable */
	struct Table {
		std::u8string id;
		constexpr Table() = default;
		constexpr Table(std::u8string id) : id{ id } {}
	};

	/* wasm::CanFunction */
	struct Function {
		std::u8string id;
		constexpr Function() = default;
		constexpr Function(std::u8string id) : id{ id } {}
		constexpr void local(const wasm::Local& local) {}
	};

	/* wasm::CanModule */
	struct Module {
	public:
		std::u8string out;

	public:
		constexpr Module(TextWriter& state) {}

	public:
		constexpr TextWriter::Memory memory(const std::u8string_view& id, const wasm::Limit& limit, const wasm::Exchange& exchange) {
			str::FormatTo(out, u8"\n  (memory{}{}{})", fMakeId(id), fMakeExchange(exchange), fMakeLimit(limit));
			return TextWriter::Memory{ std::u8string(id) };
		}
		constexpr TextWriter::Table table(const std::u8string_view& id, bool functions, const wasm::Limit& limit, const wasm::Exchange& exchange) {
			str::FormatTo(out, u8"\n  (table{}{}{} {})", fMakeId(id), fMakeExchange(exchange), fMakeLimit(limit), functions ? u8"funcref" : u8"externref");
			return TextWriter::Table{ std::u8string(id) };
		}
		constexpr TextWriter::Function function(const std::u8string_view& id, const wasm::Prototype& prototype, const wasm::Exchange& exchange) {
			str::BuildTo(out, u8"\n  (func", fMakeId(id), fMakeExchange(exchange), fMakePrototype(prototype), u8')');
			return TextWriter::Function{ std::u8string(id) };
		}
		constexpr std::u8string toString() {
			if (out.empty())
				return u8"(module)";
			return str::Build<std::u8string>(u8"(module", out, u8"\n)");
		}
	};
};
