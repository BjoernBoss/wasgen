#pragma once

#include "wasm-common.h"

namespace wasm {
	namespace detail {
		struct TableState {
			wasm::Import imported;
			wasm::Export exported;
			wasm::Limit limit;
			std::u8string_view id;
			bool functions = false;
		};
	}

	/* describe a wasm-table object */
	class Table : public detail::ModuleMember<detail::TableState> {
		friend class wasm::Module;
	public:
		constexpr Table() = default;

	private:
		constexpr Table(wasm::Module& module, uint32_t index) : ModuleMember{ module, index } {}

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
		constexpr bool functions() const {
			return fGet()->functions;
		}
	};
}
