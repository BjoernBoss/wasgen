#pragma once

#include "wasm-common.h"
#include "wasm-prototype.h"

namespace wasm {
	namespace detail {
		struct FunctionState {
			std::u8string importModule;
			std::u8string_view id;
			wasm::Prototype prototype;
			wasm::Sink* sink = 0;
			bool exported = false;
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
		constexpr bool imported() const {
			return !fGet()->importModule.empty();
		}
		constexpr bool exported() const {
			return fGet()->exported;
		}
		constexpr const std::u8string& importModule() const {
			return fGet()->importModule;
		}
		constexpr wasm::Prototype prototype() const {
			return fGet()->prototype;
		}
	};
}
