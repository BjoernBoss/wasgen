#pragma once

#include "wasm-common.h"

namespace wasm {
	namespace detail {
		struct VariableState {
			std::u8string_view name;
			wasm::Type type = wasm::Type::i32;
		};
	}

	/* describe a wasm-local or parameter of a sink */
	class Variable : public detail::SinkMember<detail::VariableState> {
		friend class wasm::Sink;
	public:
		constexpr Variable() = default;

	private:
		constexpr Variable(wasm::Sink& sink, uint32_t index) : SinkMember{ sink, index } {}

	public:
		constexpr bool valid() const {
			return (pSink != 0);
		}
		constexpr uint32_t index() const {
			return pIndex;
		}
		constexpr std::u8string_view name() const {
			return fGet()->name;
		}
		constexpr wasm::Type type() const {
			return fGet()->type;
		}
	};
}
