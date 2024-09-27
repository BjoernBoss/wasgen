#pragma once

#include "wasm-common.h"

namespace wasm {
	namespace detail {
		struct GlobalState {
			wasm::Import imported;
			wasm::Export exported;
			std::u8string_view id;
			wasm::Type type = wasm::Type::i32;
			bool mutating = false;
		};
	}

	/* describe a wasm-global object */
	class Global : public detail::ModuleMember<detail::GlobalState> {
		friend class wasm::Module;
	public:
		constexpr Global() = default;

	private:
		constexpr Global(wasm::Module& module, uint32_t index) : ModuleMember{ module, index } {}

	public:
		constexpr const wasm::Import imported() const {
			return fGet()->imported;
		}
		constexpr const wasm::Export exported() const {
			return fGet()->exported;
		}
		constexpr wasm::Type type() const {
			return fGet()->type;
		}
		constexpr bool mutating() const {
			return fGet()->mutating;
		}
	};
}
