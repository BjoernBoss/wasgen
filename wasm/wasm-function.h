#pragma once

#include "wasm-common.h"
#include "wasm-prototype.h"

namespace wasm {
	namespace detail {
		struct FunctionState {
			wasm::Import imported;
			wasm::Export exported;
			std::u8string_view id;
			wasm::Prototype prototype;
			wasm::Sink* sink = 0;
			bool bound = false;
		};
	}

	/* describe a wasm-function object */
	class Function : public detail::ModuleMember<detail::FunctionState> {
		friend class wasm::Module;
	public:
		constexpr Function() = default;

	private:
		constexpr Function(wasm::Module& module, uint32_t index) : ModuleMember{ module, index } {}

	public:
		constexpr const wasm::Import& imported() const {
			return fGet()->imported;
		}
		constexpr const wasm::Export& exported() const {
			return fGet()->exported;
		}
		constexpr wasm::Prototype prototype() const {
			return fGet()->prototype;
		}
	};
}
