#pragma once

#include "wasm-common.h"

namespace wasm {
	/* writer must provide type Memory, which the wasm::Memory internally instantiates */
	template <class Type>
	concept IsMemory = requires() {
		typename Type::Memory;
	};

	/* any memory object to be referenced by instructions [instantiated via wasm::Module] */
	template <wasm::IsMemory Writer>
	struct Memory {
		Writer::Memory self;
		constexpr Memory(Writer::Memory&& self) : self{ std::move(self) } {}
		constexpr Memory(const wasm::Memory<Writer>&) = default;
		constexpr Memory(wasm::Memory<Writer>&&) = default;
	};

	/* writer must provide type Table, which the wasm::Table internally instantiates */
	template <class Type>
	concept IsTable = requires() {
		typename Type::Table;
	};

	/* any table object to be referenced by instructions [instantiated via wasm::Module] */
	template <wasm::IsTable Writer>
	struct Table {
		Writer::Table self;
		constexpr Table(Writer::Table&& self) : self{ std::move(self) } {}
		constexpr Table(const wasm::Table<Writer>&) = default;
		constexpr Table(wasm::Table<Writer>&&) = default;
	};

	/* writer must provide type Global, which the wasm::Global internally instantiates */
	template <class Type>
	concept IsGlobal = requires() {
		typename Type::Global;
	};

	/* any global variable to be referenced by instructions [instantiated via wasm::Module] */
	template <wasm::IsGlobal Writer>
	struct Global {
		Writer::Global self;
		constexpr Global(Writer::Global&& self) : self{ std::move(self) } {}
		constexpr Global(const wasm::Global<Writer>&) = default;
		constexpr Global(wasm::Global<Writer>&&) = default;
	};

	/* writer must provide type Function, which the wasm::Function internally instantiates */
	template <class Type>
	concept IsFunction = requires() {
		typename Type::Function;
	};

	/* any function to be referenced by instructions [instantiated via wasm::Module] */
	template <wasm::IsFunction Writer>
	struct Function {
		Writer::Function self;
		constexpr Function(Writer::Function&& self) : self{ std::move(self) } {}
		constexpr Function(const wasm::Function<Writer>&) = default;
		constexpr Function(wasm::Function<Writer>&&) = default;
	};

	/* writer must provide type Local, which the wasm::Local internally instantiates */
	template <class Type>
	concept IsLocal = requires() {
		typename Type::Local;
	};

	/* any local variable to be referenced by instructions [instantiated via wasm::Sink] */
	template <wasm::IsLocal Writer>
	struct Local {
		Writer::Local self;
		constexpr Local(Writer::Local&& self) : self{ std::move(self) } {}
		constexpr Local(const wasm::Local<Writer>&) = default;
		constexpr Local(wasm::Local<Writer>&&) = default;
	};

	/* writer must provide type Instruction, which the wasm::Instruction internally instantiates */
	template <class Type>
	concept IsInstruction = requires() {
		typename Type::Instruction;
	};

	/* any non-control-flow instruction */
	template <wasm::IsInstruction Writer>
	struct Instruction {
		Writer::Instruction self;
		constexpr Instruction(Writer::Instruction&& self) : self{ std::move(self) } {}
		constexpr Instruction(const wasm::Instruction<Writer>&) = default;
		constexpr Instruction(wasm::Instruction<Writer>&&) = default;
	};

	/* writer must provide type Target, which the wasm::Target internally instantiates */
	template <class Type>
	concept IsTarget = requires() {
		typename Type::Target;
	};

	/* target to be jumped to [instantiated via wasm::Sink] */
	template <wasm::IsTarget Writer>
	struct Target {
		Writer::Target self;
		constexpr Target(Writer::Target&& self) : self{ std::move(self) } {}
		Target(wasm::Target<Writer>&&) = delete;
		Target(const wasm::Target<Writer>&) = delete;
	};
}
