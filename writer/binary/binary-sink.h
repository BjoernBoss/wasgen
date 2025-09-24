/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2024-2025 Bjoern Boss Henrichsen */
#pragma once

#include "binary-base.h"

namespace wasm::binary {
	class Sink final : public wasm::SinkInterface {
		friend class binary::Module;
	private:
		struct Local {
			uint32_t count = 0;
			wasm::Type type = wasm::Type::i32;
		};

	private:
		binary::Module* pModule = 0;
		std::vector<Local> pLocals;
		std::vector<uint8_t> pCode;
		uint32_t pIndex = 0;

	private:
		Sink(binary::Module* module, uint32_t index);

	private:
		void fPush(uint8_t byte);
		void fPush(std::initializer_list<uint8_t> bytes);
		void fPushWidth(bool _32, uint8_t i32, uint8_t i64);
		void fPushSelect(wasm::OpType operand, uint8_t i32, uint8_t i64, uint8_t f32, uint8_t f64);

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
