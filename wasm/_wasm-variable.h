#pragma once

#include "_wasm-common.h"

namespace wasm {
	namespace detail {
		struct VariableState {
			std::u8string_view name;
			wasm::_Type type = wasm::_Type::i32;
		};
	}

	/* describe a wasm-local or parameter of a sink */
	class _Variable : public detail::SinkMember<detail::VariableState> {
		friend class wasm::_Sink;
	public:
		constexpr _Variable() = default;

	private:
		constexpr _Variable(wasm::_Sink& sink, uint32_t index) : SinkMember{ sink, index } {}

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
		constexpr wasm::_Type type() const {
			return fGet()->type;
		}
	};
}
