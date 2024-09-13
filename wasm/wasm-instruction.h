#pragma once

#include "wasm-module.h"

namespace wasm {
	enum class NoOpInst : uint8_t {
		add,
		ret
	};

	template <class Writer>
	struct Inst {
		static constexpr wasm::Instruction<Writer> Add() {
			return wasm::Instruction<Writer>{ std::move(Writer::NoOpInst(wasm::NoOpInst::add)) };
		}
		static constexpr wasm::Instruction<Writer> Return() {
			return wasm::Instruction<Writer>{ std::move(Writer::NoOpInst(wasm::NoOpInst::ret)) };
		}
		static constexpr wasm::Instruction<Writer> Const(uint32_t v) {
			return wasm::Instruction<Writer>{ std::move(Writer::InstConst(v)) };
		}
	};
}
