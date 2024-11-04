/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2024 Bjoern Boss Henrichsen */
#pragma once

#include "../wasm-common.h"
#include "../objects/wasm-prototype.h"

namespace wasm {
	enum class ScopeType : uint8_t {
		conditional,
		loop,
		block
	};

	namespace detail {
		struct TargetState {
			wasm::Prototype prototype;
			std::u8string id;
			size_t stamp = 0;
			wasm::ScopeType type = wasm::ScopeType::conditional;
			bool otherwise = false;
		};
	}

	/* describe a wasm-target to be jumped to for a sink (will implicitly be closed at destruction) */
	class Target : public detail::SinkMember<detail::TargetState> {
		friend class wasm::Sink;
	private:
		size_t pStamp = 0;

	public:
		Target();
		Target(wasm::Target&&) noexcept;
		Target(const wasm::Target&) = delete;
		~Target() noexcept(false);

	protected:
		Target(wasm::Sink& sink);

	protected:
		void fSetup(std::u8string_view label, const wasm::Prototype& prototype, wasm::ScopeType type);
		void fSetup(std::u8string_view label, std::initializer_list<wasm::Type> params, std::initializer_list<wasm::Type> result, wasm::ScopeType type);
		void fToggle();
		void fClose();

	public:
		void close();
		bool valid() const;
		uint32_t index() const;
		std::u8string_view id() const;
		wasm::Prototype prototype() const;
		wasm::ScopeType type() const;
		std::u8string toString() const;
	};

	/* create a conditional if/then/else block, which can be jumped to for a sink */
	struct IfThen : public wasm::Target {
		IfThen(wasm::Sink& sink, std::u8string_view label, const wasm::Prototype& prototype);
		IfThen(wasm::Sink& sink, std::u8string_view label = {}, std::initializer_list<wasm::Type> params = {}, std::initializer_list<wasm::Type> result = {});
		void otherwise();
	};

	/* create a loop block, which can be jumped to for a sink */
	struct Loop : public wasm::Target {
		Loop(wasm::Sink& sink, std::u8string_view label, const wasm::Prototype& prototype);
		Loop(wasm::Sink& sink, std::u8string_view label = {}, std::initializer_list<wasm::Type> params = {}, std::initializer_list<wasm::Type> result = {});
	};

	/* create a jump block, which can be jumped to for a sink */
	struct Block : public wasm::Target {
		Block(wasm::Sink& sink, std::u8string_view label, const wasm::Prototype& prototype);
		Block(wasm::Sink& sink, std::u8string_view label = {}, std::initializer_list<wasm::Type> params = {}, std::initializer_list<wasm::Type> result = {});
	};
}
