#pragma once

#include "wasm-common.h"
#include "wasm-prototype.h"

namespace wasm {
	enum class ScopeType : uint8_t {
		conditional,
		loop,
		block
	};

	namespace detail {
		struct TargetState {
			wasm::Prototype prototype;
			std::u8string label;
			size_t stamp = 0;
			wasm::ScopeType type = wasm::ScopeType::conditional;
			bool otherwise = false;
		};
	}

	/* describe a wasm-target to be jumped to for a sink */
	class Target : public detail::SinkMember<detail::TargetState> {
		friend class wasm::Sink;
	private:
		size_t pStamp = 0;

	public:
		Target() = delete;
		Target(wasm::Target&&) = delete;
		Target(const wasm::Target&) = delete;

	protected:
		Target(wasm::Sink& sink);

	protected:
		void fSetup(const wasm::Prototype& prototype, std::u8string_view label, wasm::ScopeType type);
		void fToggle();
		void fClose();

	public:
		bool valid() const;
		uint32_t index() const;
		std::u8string_view label() const;
		wasm::Prototype prototype() const;
		wasm::ScopeType type() const;
	};

	/* create a conditional if/then/else block, which can be jumped to for a sink */
	struct IfThen : public wasm::Target {
		IfThen(wasm::Sink& sink, std::u8string_view label = {}, const wasm::Prototype& prototype = {});
		~IfThen();
		void close();
		void otherwise();
	};

	/* create a loop block, which can be jumped to for a sink */
	struct Loop : public wasm::Target {
		Loop(wasm::Sink& sink, std::u8string_view label = {}, const wasm::Prototype& prototype = {});
		~Loop();
		void close();
	};

	/* create a jump block, which can be jumped to for a sink */
	struct Block : public wasm::Target {
		Block(wasm::Sink& sink, std::u8string_view label = {}, const wasm::Prototype& prototype = {});
		~Block();
		void close();
	};
}
