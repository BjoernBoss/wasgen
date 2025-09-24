/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2024-2025 Bjoern Boss Henrichsen */
#pragma once

#include "../wasm-common.h"

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
		explicit constexpr Global() = default;

	private:
		explicit constexpr Global(wasm::Module& module, uint32_t index) : ModuleMember{ module, index } {}

	public:
		constexpr bool imported() const;
		constexpr bool exported() const;
		constexpr const std::u8string& importModule() const;
		constexpr wasm::Type type() const;
		constexpr bool mutating() const;
		constexpr bool assigned() const;
	};
}
