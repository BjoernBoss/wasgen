/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2024 Bjoern Boss Henrichsen */
#pragma once

#include "../wasm-common.h"
#include "../objects/wasm-function.h"
#include "../objects/wasm-global.h"
#include "../inst/wasm-instruction.h"
#include "wasm-variable.h"
#include "wasm-target.h"

namespace wasm {
	/* sink interface used to write the instructions out */
	class SinkInterface {
	public:
		virtual void pushScope(const wasm::Target& target) = 0;
		virtual void popScope(wasm::ScopeType type) = 0;
		virtual void toggleConditional() = 0;
		virtual void close(const wasm::Sink& sink) = 0;
		virtual void addLocal(const wasm::Variable& local) = 0;
		virtual void addInst(const wasm::InstSimple& inst) = 0;
		virtual void addInst(const wasm::InstConst& inst) = 0;
		virtual void addInst(const wasm::InstOperand& inst) = 0;
		virtual void addInst(const wasm::InstWidth& inst) = 0;
		virtual void addInst(const wasm::InstMemory& inst) = 0;
		virtual void addInst(const wasm::InstTable& inst) = 0;
		virtual void addInst(const wasm::InstLocal& inst) = 0;
		virtual void addInst(const wasm::InstGlobal& inst) = 0;
		virtual void addInst(const wasm::InstFunction& inst) = 0;
		virtual void addInst(const wasm::InstIndirect& inst) = 0;
		virtual void addInst(const wasm::InstBranch& inst) = 0;
	};

	/* write instructions out to a function bound to the given sink out to the sink-implementation */
	class Sink {
		template <class> friend class detail::SinkMember;
		friend class wasm::Module;
		friend class wasm::Target;
	private:
		struct LocalList {
			wasm::Sink* _this = 0;
			constexpr LocalList(wasm::Sink* module) : _this{ module } {}
			constexpr size_t size() const {
				return _this->pVariables.list.size() - _this->pParameter;
			}
			constexpr wasm::Variable get(uint32_t index) const {
				return wasm::Variable{ *_this, index + _this->pParameter };
			}
		};

	private:
		wasm::Module* pModule = 0;
		struct {
			std::vector<detail::VariableState> list;
			std::unordered_set<std::u8string> ids;
		} pVariables;
		std::vector<detail::TargetState> pTargets;
		wasm::Function pFunction;
		wasm::SinkInterface* pInterface = 0;
		size_t pNextStamp = 0;
		uint32_t pParameter = 0;
		bool pClosed = false;

	public:
		Sink(const wasm::Function& function);
		Sink() = delete;
		Sink(wasm::Sink&&) = delete;
		Sink(const wasm::Sink&) = delete;
		~Sink();

	private:
		std::u8string fError() const;
		void fClose();
		void fCheckClosed() const;
		void fPopUntil(uint32_t size);
		bool fCheckTarget(uint32_t index, size_t stamp, bool soft) const;
		void fSetupValidTarget(const wasm::Prototype& prototype, std::u8string_view id, wasm::ScopeType type, wasm::Target& target);
		void fSetupTarget(const wasm::Prototype& prototype, std::u8string_view id, wasm::ScopeType type, wasm::Target& target);
		void fSetupTarget(std::initializer_list<wasm::Type> params, std::initializer_list<wasm::Type> result, std::u8string_view id, wasm::ScopeType type, wasm::Target& target);
		void fToggleTarget(uint32_t index, size_t stamp);
		void fCloseTarget(uint32_t index, size_t stamp);

	public:
		wasm::Variable parameter(uint32_t index);
		wasm::Variable local(wasm::Type type, std::u8string_view id = {});
		wasm::Function function() const;
		void close();

	public:
		wasm::List<wasm::Variable, Sink::LocalList> locals() const;

	public:
		void operator[](const wasm::InstSimple& inst);
		void operator[](const wasm::InstConst& inst);
		void operator[](const wasm::InstOperand& inst);
		void operator[](const wasm::InstWidth& inst);
		void operator[](const wasm::InstMemory& inst);
		void operator[](const wasm::InstTable& inst);
		void operator[](const wasm::InstLocal& inst);
		void operator[](const wasm::InstGlobal& inst);
		void operator[](const wasm::InstFunction& inst);
		void operator[](const wasm::InstIndirect& inst);
		void operator[](const wasm::InstBranch& inst);
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
