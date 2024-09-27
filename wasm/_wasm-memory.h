#pragma once

#include "_wasm-common.h"

namespace wasm {
	namespace detail {
		struct MemoryState {
			wasm::_Import imported;
			wasm::_Export exported;
			wasm::_Limit limit;
			std::u8string_view id;
		};
	}

	class _Memory : public detail::ModuleMember<detail::MemoryState> {
		friend class wasm::_Module;
	public:
		constexpr _Memory() = default;

	private:
		constexpr _Memory(wasm::_Module& module, uint32_t index) : ModuleMember{ module, index } {}

	public:
		constexpr const wasm::_Import& imported() const {
			return fGet()->imported;
		}
		constexpr const wasm::_Export& exported() const {
			return fGet()->exported;
		}
		constexpr const wasm::_Limit& limit() const {
			return fGet()->limit;
		}
	};
}
