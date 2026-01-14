/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2024-2026 Bjoern Boss Henrichsen */
#pragma once

#include "binary-base.h"

namespace wasm::binary {
	class Module final : public wasm::ModuleInterface {
		friend class binary::Sink;
	private:
		struct Section {
			std::vector<uint8_t> buffer;
			uint32_t count = 0;
		};
		struct Deferred {
			std::vector<std::vector<uint8_t>> data;
			uint32_t indexOffset = 0;
		};

	private:
		Section pPrototype;
		Section pFunction;
		Section pImport;
		Section pExport;
		Deferred pTable;
		Deferred pMemory;
		Section pElement;
		Section pData;
		Section pStart;
		Deferred pCode;
		Deferred pGlobal;
		std::vector<uint8_t> pOutput;

	private:
		void fWriteImport(const std::u8string& importModule, std::u8string_view id, uint8_t type);
		void fWriteExport(std::u8string_view id, uint8_t type);
		void fWriteSection(const Section& section, bool placeCount, uint8_t id);
		void fWriteSection(const Deferred& section, bool placeSlotSize, uint8_t id);

	public:
		const std::vector<uint8_t>& output() const;

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
