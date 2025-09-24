/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2024-2025 Bjoern Boss Henrichsen */
#pragma once

#include "../wasm-common.h"

namespace wasm {
	namespace detail {
		struct VariableState {
			std::u8string_view id;
			wasm::Type type = wasm::Type::i32;
		};
	}

	/* describe a wasm-local or parameter of a sink */
	class Variable : public detail::SinkMember<detail::VariableState> {
		friend class wasm::Sink;
	public:
		constexpr Variable() = default;

	private:
		constexpr Variable(wasm::Sink& sink, uint32_t index) : SinkMember{ sink, index } {}

	public:
		constexpr bool valid() const {
			return (pSink != 0);
		}
		constexpr uint32_t index() const {
			return pIndex;
		}
		constexpr std::u8string_view id() const;
		constexpr wasm::Type type() const;
		std::u8string toString() const;
	};
}
