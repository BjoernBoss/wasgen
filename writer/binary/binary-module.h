#pragma once

#include "binary-base.h"

namespace writer::binary {
	class Module final : public wasm::ModuleInterface {
		friend class binary::Sink;
	private:
		struct Section {
			std::vector<uint8_t> buffer;
			uint32_t count = 0;
		};

	private:
		Section pPrototype;
		Section pFunction;
		Section pImport;
		Section pExport;
		Section pGlobal;
		Section pTable;
		Section pMemory;
		std::vector<std::vector<uint8_t>> pCode;
		std::vector<uint8_t> pOutput;
		uint32_t pCodeOffset = 0;

	private:
		void fWriteImport(const wasm::Import& imported, uint8_t type);
		void fWriteExport(const wasm::Export& exported, uint8_t type);
		void fWriteSection(const Section& section, uint32_t size, uint8_t id);

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
	};
}
