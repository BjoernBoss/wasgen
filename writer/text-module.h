#pragma once

#include "text-base.h"

namespace writer::text {
	class Module final : public wasm::ModuleInterface {
		friend class text::Sink;
	private:
		std::vector<std::u8string> pFunctions;
		std::u8string pImports;
		std::u8string pDefined;
		std::u8string pOutput;

	public:
		const std::u8string& output() const;

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
