#pragma once

#include "_wasm-common.h"

namespace wasm {
	namespace detail {
		struct GlobalState {
			wasm::_Import imported;
			wasm::_Export exported;
			std::u8string_view id;
			wasm::_Type type = wasm::_Type::i32;
			bool mutating = false;
		};
	}

	class _Global : public detail::ModuleMember<detail::GlobalState> {
		friend class wasm::_Module;
	public:
		constexpr _Global() = default;

	private:
		constexpr _Global(wasm::_Module& module, uint32_t index) : ModuleMember{ module, index } {}

	public:
		constexpr const wasm::_Import& imported() const {
			return fGet()->imported;
		}
		constexpr const wasm::_Export& exported() const {
			return fGet()->exported;
		}
		constexpr wasm::_Type type() const {
			return fGet()->type;
		}
		constexpr bool mutating() const {
			return fGet()->mutating;
		}
	};
}
