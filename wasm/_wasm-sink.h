#pragma once

#include "_wasm-common.h"
#include "_wasm-variable.h"
#include "_wasm-target.h"
#include "_wasm-function.h"
#include "_wasm-instruction.h"

namespace wasm {
	class _SinkInterface {
	public:
		virtual void pushScope(const wasm::_Target& target) = 0;
		virtual void popScope() = 0;
		virtual void toggleConditional() = 0;
		virtual void close() = 0;
		virtual void addLocal(const wasm::_Variable& local) = 0;
		virtual void addInst(const wasm::_InstConst& inst) = 0;
		virtual void addInst(const wasm::_InstSimple& inst) = 0;
		virtual void addInst(const wasm::_InstMemory& inst) = 0;
		virtual void addInst(const wasm::_InstTable& inst) = 0;
		virtual void addInst(const wasm::_InstLocal& inst) = 0;
		virtual void addInst(const wasm::_InstGlobal& inst) = 0;
		virtual void addInst(const wasm::_InstFunction& inst) = 0;
		virtual void addInst(const wasm::_InstIndirect& inst) = 0;
		virtual void addInst(const wasm::_InstBranch& inst) = 0;
	};

	class _Sink {
		template <class> friend class detail::SinkMember;
		friend class wasm::_Target;
	private:
		struct _LocalList {
			wasm::_Sink* _this = 0;
			constexpr _LocalList(wasm::_Sink* module) : _this{ module } {}
			constexpr size_t size() const {
				return _this->pVariables.list.size() - _this->pParameter;
			}
			constexpr wasm::_Variable get(uint32_t index) const {
				return wasm::_Variable{ *_this, index + _this->pParameter };
			}
		};

	private:
		struct {
			std::vector<detail::VariableState> list;
			std::unordered_set<std::u8string> names;
		} pVariables;
		std::vector<detail::TargetState> pTargets;
		wasm::_Function pFunction;
		wasm::_SinkInterface* pInterface = 0;
		size_t pNextStamp = 0;
		uint32_t pParameter = 0;
		bool pClosed = false;

	public:
		_Sink(const wasm::_Function& function);
		_Sink() = delete;
		_Sink(wasm::_Sink&&) = delete;
		_Sink(const wasm::_Sink&) = delete;
		~_Sink();

	private:
		void fClose();
		void fCheckClosed();
		void fPopUntil(uint32_t size);
		bool fCheckTarget(uint32_t index, size_t stamp, bool soft) const;
		void fSetupTarget(const wasm::_Prototype& prototype, std::u8string_view label, wasm::_ScopeType type, wasm::_Target& target);
		void fToggleTarget(uint32_t index, size_t stamp);
		void fCloseTarget(uint32_t index, size_t stamp);

	public:
		wasm::_Variable parameter(uint32_t index);
		wasm::_Variable local(wasm::_Type type, std::u8string_view name);
		wasm::_Function function() const;
		void close();

	public:
		wasm::_List<wasm::_Variable, _Sink::_LocalList> locals() const;

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
