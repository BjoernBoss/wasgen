#pragma once

#include "_wasm-common.h"
#include "_wasm-variable.h"
#include "_wasm-target.h"
#include "_wasm-function.h"
#include "_wasm-instruction.h"

namespace wasm {
	class _Sink {
		template <class> friend class detail::SinkMember;
		friend class wasm::_Target;
	private:
		struct {
			std::vector<detail::VariableState> list;
			std::unordered_set<std::u8string> names;
		} pVariables;
		std::vector<detail::TargetState> pTargets;
		wasm::_Function pFunction;
		size_t pNextStamp = 0;
		uint32_t pParameter = 0;

	private:
		void fCloseUntil(uint32_t size);
		bool fCheckTarget(uint32_t index, size_t stamp, bool soft) const;
		uint32_t fSetupTarget(const wasm::_Prototype& prototype, std::u8string_view label, detail::TargetType type);
		void fToggleTarget(uint32_t index, size_t stamp);
		void fCloseTarget(uint32_t index, size_t stamp);

	public:
		wasm::_Variable parameter(uint32_t index);
		wasm::_Variable local(wasm::_Type type, std::u8string_view name);
		wasm::_Function function() const;

	public:
		void operator[](const wasm::_InstConst& inst);
		void operator[](const wasm::_InstSimple& inst);
		void operator[](const wasm::_InstMemory& inst);
		void operator[](const wasm::_InstTable& inst);
		void operator[](const wasm::_InstLocal& inst);
		void operator[](const wasm::_InstGlobal& inst);
		void operator[](const wasm::_InstFunction& inst);
		void operator[](const wasm::_InstIndirect& inst);
		void operator[](const wasm::_InstBranch& inst);
	};

	namespace detail {
		template <class Type>
		constexpr const Type* SinkMember<Type>::fGet() const {
			if constexpr (std::is_same_v<Type, detail::VariableState>)
				return &pSink->pVariables.list[pIndex];
			if constexpr (std::is_same_v<Type, detail::TargetState>)
				return &pSink->pTargets[pIndex];
			return nullptr;
		}
	}
}
