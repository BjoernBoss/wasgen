/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2024 Bjoern Boss Henrichsen */
#pragma once

#include "text-base.h"

namespace wasm::text {
	class Sink final : public wasm::SinkInterface {
		friend class text::Module;
	private:
		text::Module* pModule = 0;
		std::u8string pLocals;
		std::u8string pBody;
		std::u8string pDepth;

	private:
		Sink(text::Module* module, std::u8string header);

	private:
		void fAddLine(std::u8string_view str);
		void fPush(std::u8string_view name);
		void fPop();

	public:
		void pushScope(const wasm::Target& target) override;
		void popScope(wasm::ScopeType type) override;
		void toggleConditional() override;
		void close(const wasm::Sink& sink) override;
		void addLocal(const wasm::Variable& local) override;
		void addComment(std::u8string_view text) override;
		void addInst(const wasm::InstSimple& inst) override;
		void addInst(const wasm::InstConst& inst) override;
		void addInst(const wasm::InstOperand& inst) override;
		void addInst(const wasm::InstWidth& inst) override;
		void addInst(const wasm::InstMemory& inst) override;
		void addInst(const wasm::InstTable& inst) override;
		void addInst(const wasm::InstLocal& inst) override;
		void addInst(const wasm::InstGlobal& inst) override;
		void addInst(const wasm::InstFunction& inst) override;
		void addInst(const wasm::InstIndirect& inst) override;
		void addInst(const wasm::InstBranch& inst) override;
	};
}
