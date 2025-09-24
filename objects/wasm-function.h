/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2024-2025 Bjoern Boss Henrichsen */
#pragma once

#include "../wasm-common.h"
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
		constexpr bool imported() const;
		constexpr bool exported() const;
		constexpr const std::u8string& importModule() const;
		constexpr wasm::Prototype prototype() const;
	};
}
