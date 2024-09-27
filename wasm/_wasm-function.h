#pragma once

#include "_wasm-common.h"
#include "_wasm-prototype.h"

namespace wasm {
	namespace detail {
		struct FunctionState {
			wasm::_Import imported;
			wasm::_Export exported;
			std::u8string_view id;
			wasm::_Prototype prototype;
			wasm::_Sink* sink = 0;
			bool bound = false;
		};
	}

	/* describe a wasm-function object */
	class _Function : public detail::ModuleMember<detail::FunctionState> {
		friend class wasm::_Module;
	public:
		constexpr _Function() = default;

	private:
		constexpr _Function(wasm::_Module& module, uint32_t index) : ModuleMember{ module, index } {}

	public:
		constexpr const wasm::_Import& imported() const {
			return fGet()->imported;
		}
		constexpr const wasm::_Export& exported() const {
			return fGet()->exported;
		}
		constexpr wasm::_Prototype prototype() const {
			return fGet()->prototype;
		}
	};
}
