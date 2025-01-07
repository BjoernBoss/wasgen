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
		virtual void addComment(std::u8string_view text) = 0;
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
		struct Scope {
			size_t stack = 0;
			bool unreachable = false;
		};
		struct Scopes {
			detail::TargetState state;
			Scope scope;
		};

	private:
		wasm::Module* pModule = 0;
		struct {
			std::vector<detail::VariableState> list;
			std::unordered_set<std::u8string> ids;
		} pVariables;
		std::vector<Scopes> pTargets;
		std::vector<wasm::Type> pStack;
		Scope pRoot;
		wasm::Function pFunction;
		wasm::SinkInterface* pInterface = 0;
		mutable std::wstring pException;
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
		wasm::Type fMapOperand(wasm::OpType operand) const;
		std::u8string fError() const;
		void fCheck() const;
		void fClose();
		void fDeferredException(const wasm::Exception& error);
		wasm::Variable fParam(uint32_t index);

	private:
		void fPopUntil(uint32_t size);
		bool fCheckTarget(uint32_t index, size_t stamp, bool soft) const;
		void fSetupValidTarget(const wasm::Prototype& prototype, std::u8string_view id, wasm::ScopeType type, wasm::Target& target);
		void fSetupTarget(const wasm::Prototype& prototype, std::u8string_view id, wasm::ScopeType type, wasm::Target& target);
		void fSetupTarget(std::vector<wasm::Type> params, std::vector<wasm::Type> result, std::u8string_view id, wasm::ScopeType type, wasm::Target& target);
		void fToggleTarget(uint32_t index, size_t stamp);
		void fCloseTarget(uint32_t index, size_t stamp);

	private:
		template <class ItType, class MkType>
		std::wstring fMakeTypeList(ItType begin, ItType end, MkType make) const {
			std::wstring expected;

			/* create the comma-separated list of types */
			while (begin != end) {
				if (!expected.empty())
					expected.append(L", ");

				/* append the type */
				wasm::Type type = make(*begin);
				switch (type) {
				case wasm::Type::i32:
					expected.append(L"i32");
					break;
				case wasm::Type::i64:
					expected.append(L"i64");
					break;
				case wasm::Type::f32:
					expected.append(L"f32");
					break;
				case wasm::Type::f64:
					expected.append(L"f64");
					break;
				case wasm::Type::refExtern:
					expected.append(L"externref");
					break;
				case wasm::Type::refFunction:
					expected.append(L"funcref");
					break;
				default:
					throw wasm::Exception{ L"Unknown wasm type [", size_t(type), L"] encountered" };
				}
				++begin;
			}
			return expected;
		}

	private:
		void fTypesFailed(std::wstring_view expected, std::wstring_view found) const;
		void fPopFailed(size_t count, std::wstring_view expected) const;
		void fCheckEmpty() const;
		const Scope& fScope() const;
		Scope& fScope();
		void fPopTypes(const wasm::Prototype& prototype, bool params);
		void fPopTypes(std::initializer_list<wasm::Type> types);
		void fSwapTypes(std::initializer_list<wasm::Type> pop, std::initializer_list<wasm::Type> push);
		void fPushTypes(std::initializer_list<wasm::Type> types);
		void fPushTypes(const wasm::Prototype& prototype, bool params);

	public:
		wasm::Variable param(uint32_t index);
		wasm::Variable local(wasm::Type type, std::u8string_view id = {});
		void comment(std::u8string_view text);
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
		void operator[](const wasm::InstParam& inst);
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
				return &pSink->pTargets[pIndex].state;
			return nullptr;
		}
	}

	constexpr std::u8string_view wasm::Variable::id() const {
		return fGet()->id;
	}
	constexpr wasm::Type wasm::Variable::type() const {
		return fGet()->type;
	}
}
