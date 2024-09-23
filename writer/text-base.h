#pragma once

#include <ustring/ustring.h>
#include <algorithm>
#include <unordered_set>
#include <vector>

#include "../wasm/wasm.h"
#include "../util/logging.h"

namespace writer {
	struct TextWriter;

	namespace text {
		struct IdObject {
			std::u8string id;
			uint32_t index = 0;
			constexpr IdObject(std::u8string id, uint32_t index) : id{ id }, index{ index } {}
			constexpr std::u8string toString() const {
				if (id.empty())
					return str::Int<std::u8string>(index);
				return id;
			}
		};

		using Prototype = IdObject;

		struct Instruction {
			std::u8string string;
			constexpr Instruction(std::u8string string) : string{ string } {}
		};

		/* helper functions */
		static constexpr std::u8string MakeId(const std::u8string_view& id) {
			if (id.empty())
				return std::u8string();
			return str::Build<std::u8string>(U" $", id);
		}
		static constexpr std::u8string_view MakeType(wasm::Type type) {
			switch (type) {
			case wasm::Type::i64:
				return u8" i64";
			case wasm::Type::f32:
				return u8" f32";
			case wasm::Type::f64:
				return u8" f64";
			case wasm::Type::refExtern:
				return u8" externref";
			case wasm::Type::refFunction:
				return u8" funcref";
			case wasm::Type::v128:
				return u8" v128";
			case wasm::Type::i32:
			default:
				return u8" i32";
			}
		}
		static constexpr std::u8string MakeExchange(const wasm::Exchange& exchange) {
			std::u8string out;
			if (!exchange.expName.empty())
				str::FormatTo(out, u8" (export \"{}\")", exchange.expName);
			if (!exchange.impName.empty())
				str::FormatTo(out, u8" (import \"{}\" \"{}\")", exchange.impModule, exchange.impName);
			return out;
		}
		static constexpr std::u8string MakeLimit(const wasm::Limit& limit) {
			if (limit.max == std::numeric_limits<uint32_t>::max())
				return str::Build<std::u8string>(u8' ', limit.min);
			return str::Build<std::u8string>(u8' ', limit.min, u8' ', limit.max);
		}
	}
}
