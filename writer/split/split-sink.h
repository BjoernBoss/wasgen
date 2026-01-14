/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2024-2026 Bjoern Boss Henrichsen */
#pragma once

#include "split-base.h"

namespace wasm::split {
	class Sink final : public wasm::SinkInterface {
		friend class split::Module;
	private:
		std::vector<wasm::SinkInterface*> pSinks;

	private:
		Sink(std::vector<wasm::SinkInterface*>&& sinks);

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
