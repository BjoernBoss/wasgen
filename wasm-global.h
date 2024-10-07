/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2024 Bjoern Boss Henrichsen */
#pragma once

#include "wasm-common.h"

namespace wasm {
	namespace detail {
		struct GlobalState {
			std::u8string importModule;
			std::u8string_view id;
			wasm::Type type = wasm::Type::i32;
			bool exported = false;
			bool mutating = false;
			bool assigned = false;
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
		constexpr bool imported() const {
			return !fGet()->importModule.empty();
		}
		constexpr bool exported() const {
			return fGet()->exported;
		}
		constexpr const std::u8string& importModule() const {
			return fGet()->importModule;
		}
		constexpr wasm::Type type() const {
			return fGet()->type;
		}
		constexpr bool mutating() const {
			return fGet()->mutating;
		}
	};
}
