/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2024 Bjoern Boss Henrichsen */
#pragma once

#include "../wasm-common.h"

namespace wasm {
	namespace detail {
		struct TableState {
			std::u8string importModule;
			wasm::Limit limit;
			std::u8string_view id;
			bool exported = false;
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
		constexpr bool imported() const {
			return !fGet()->importModule.empty();
		}
		constexpr bool exported() const {
			return fGet()->exported;
		}
		constexpr const std::u8string& importModule() const {
			return fGet()->importModule;
		}
		constexpr const wasm::Limit& limit() const {
			return fGet()->limit;
		}
		constexpr bool functions() const {
			return fGet()->functions;
		}
	};
}
