#pragma once

#include "wasm.h"

#include <ustring/str.h>
#include <algorithm>

struct TextWriter {
private:
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

public:
	/* wasm::CanMemory */
	struct Memory {
		std::u8string out;
		constexpr Memory(TextWriter& state, const std::u8string_view& id, const wasm::Limit& limit, const wasm::Exchange& exchange) {
			str::FormatTo(out, u8"(memory{}{}{})", fMakeId(id), fMakeExchange(exchange), fMakeLimit(limit));
		}
	};

	/* wasm::CanTable */
	struct Table {
		std::u8string out;
		constexpr Table(TextWriter& state, const std::u8string_view& id, bool functions, const wasm::Limit& limit, const wasm::Exchange& exchange) {
			str::FormatTo(out, u8"(table{}{}{} {})", fMakeId(id), fMakeExchange(exchange), fMakeLimit(limit), functions ? u8"funcref" : u8"externref");
		}
	};

	/* wasm::CanModule */
	struct Module {
		std::u8string out;

		constexpr Module(TextWriter& state) {}
		constexpr void addMemory(const TextWriter::Memory& memory) {
			str::BuildTo(out, u8"\n  ", memory.out);
		}
		constexpr void addTable(const TextWriter::Table& table) {
			str::BuildTo(out, u8"\n  ", table.out);
		}
		constexpr std::u8string toString() {
			if (out.empty())
				return u8"(module)";
			return str::Build<std::u8string>(u8"(module", out, u8"\n)");
		}
	};
};
