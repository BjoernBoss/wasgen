/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2024 Bjoern Boss Henrichsen */
#pragma once

#include "split-base.h"

namespace wasm::split {
	class Module final : public wasm::ModuleInterface {
		friend class split::Sink;
	private:
		std::vector<wasm::ModuleInterface*> pModules;

	public:
		Module(std::initializer_list<wasm::ModuleInterface*> modules);

	public:
		wasm::SinkInterface* sink(const wasm::Function& function) override;
		void close(const wasm::Module& module) override;
		void addPrototype(const wasm::Prototype& prototype) override;
		void addMemory(const wasm::Memory& memory) override;
		void addTable(const wasm::Table& table) override;
		void addGlobal(const wasm::Global& global) override;
		void addFunction(const wasm::Function& function) override;
		void setMemoryLimit(const wasm::Memory& memory) override;
		void setTableLimit(const wasm::Table& table) override;
		void setStartup(const wasm::Function& function) override;
		void setValue(const wasm::Global& global, const wasm::Value& value) override;
		void writeData(const wasm::Memory& memory, const wasm::Value& offset, const uint8_t* data, uint32_t count) override;
		void writeElements(const wasm::Table& table, const wasm::Value& offset, const wasm::Value* values, uint32_t count) override;
	};
}
