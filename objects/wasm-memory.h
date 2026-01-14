/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2024-2026 Bjoern Boss Henrichsen */
#pragma once

#include "../wasm-common.h"

namespace wasm {
	namespace detail {
		struct MemoryState {
			std::u8string importModule;
			wasm::Limit limit;
			std::u8string_view id;
			bool exported = false;
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
		constexpr bool imported() const;
		constexpr bool exported() const;
		constexpr const std::u8string& importModule() const;
		constexpr const wasm::Limit& limit() const;
	};
}
