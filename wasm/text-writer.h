#pragma once

#include "wasm.h"

#include <ustring/str.h>
#include <algorithm>

struct TextWriter {
	/* wasm::CanLimit */
	struct Limit {
		std::u8string out;
		constexpr Limit(TextWriter& state, uint32_t min, uint32_t max) {
			max = std::max<uint32_t>(min, max);
			if (max == std::numeric_limits<uint32_t>::max())
				str::BuildTo(out, min);
			else
				str::BuildTo(out, min, u8' ', max);
		}
	};

	/* wasm::CanMemory */
	struct Memory {
		std::u8string out;
		constexpr Memory(TextWriter& state, const std::u8string_view& id, const TextWriter::Limit& limit) {
			str::FormatTo(out, u8"(memory{}{} {})", (id.empty() ? u8"" : u8" $"), id, limit.out);
		}
	};
};
