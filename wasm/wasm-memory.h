#pragma once

#include "wasm-common.h"

namespace wasm {
	namespace detail {
		struct MemoryState {
			wasm::Import imported;
			wasm::Export exported;
			wasm::Limit limit;
			std::u8string_view id;
		};
	}

	/* describe a wasm-memory object */
	class Memory : public detail::ModuleMember<detail::MemoryState> {
		friend class wasm::Module;
	public:
		constexpr Memory() = default;

	private:
		constexpr Memory(wasm::Module& module, uint32_t index) : ModuleMember{ module, index } {}

	public:
		constexpr const wasm::Import& imported() const {
			return fGet()->imported;
		}
		constexpr const wasm::Export& exported() const {
			return fGet()->exported;
		}
		constexpr const wasm::Limit& limit() const {
			return fGet()->limit;
		}
	};
}
