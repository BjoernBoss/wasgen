#pragma once

#include "_wasm-common.h"
#include "_wasm-prototype.h"

namespace wasm {
	enum class _ScopeType : uint8_t {
		conditional,
		loop,
		block
	};

	namespace detail {
		struct TargetState {
			wasm::_Prototype prototype;
			std::u8string label;
			size_t stamp = 0;
			wasm::_ScopeType type = wasm::_ScopeType::conditional;
			bool otherwise = false;
		};
	}

	class _Target : public detail::SinkMember<detail::TargetState> {
		friend class wasm::_Sink;
	private:
		size_t pStamp = 0;

	public:
		_Target() = delete;
		_Target(wasm::_Target&&) = delete;
		_Target(const wasm::_Target&) = delete;

	protected:
		_Target(wasm::_Sink& sink);

	protected:
		void fSetup(const wasm::_Prototype& prototype, std::u8string_view label, wasm::_ScopeType type);
		void fToggle();
		void fClose();

	public:
		bool valid() const;
		uint32_t index() const;
		std::u8string_view label() const;
		wasm::_Prototype prototype() const;
		wasm::_ScopeType type() const;
	};

	struct _IfThen : public wasm::_Target {
		_IfThen(wasm::_Sink& sink, const wasm::_Prototype& prototype = {}, std::u8string_view label = {});
		~_IfThen();
		void close();
		void otherwise();
	};

	struct _Loop : public wasm::_Target {
		_Loop(wasm::_Sink& sink, const wasm::_Prototype& prototype = {}, std::u8string_view label = {});
		~_Loop();
		void close();
	};

	struct _Block : public wasm::_Target {
		_Block(wasm::_Sink& sink, const wasm::_Prototype& prototype = {}, std::u8string_view label = {});
		~_Block();
		void close();
	};
}
