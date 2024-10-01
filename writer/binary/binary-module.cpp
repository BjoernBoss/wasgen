#include "binary-module.h"

void writer::binary::Module::fWriteImport(const wasm::Import& imported, uint8_t type) {
	binary::WriteString(pImport.buffer, imported.module);
	binary::WriteString(pImport.buffer, imported.name);
	binary::Write<uint8_t>(pImport.buffer, { type });
	++pImport.count;
}
void writer::binary::Module::fWriteExport(const wasm::Export& exported, uint8_t type) {
	binary::WriteString(pExport.buffer, exported.name);
	binary::Write<uint8_t>(pExport.buffer, { type });
	++pExport.count;
}
void writer::binary::Module::fWriteSection(const Section& section, uint8_t id) {
	/* write the id out */
	binary::Write<uint8_t>(pOutput, { id });

	/* write the byte-size and count out */
	binary::Write<uint32_t>(pOutput, { uint32_t(section.buffer.size()), section.count });

	/* write the actual data out */
	pOutput.insert(pOutput.end(), section.buffer.begin(), section.buffer.end());
}

const std::vector<uint8_t>& writer::binary::Module::output() const {
	if (pOutput.empty())
		util::fail(u8"Cannot produce binary-writer module output before the wrapping wasm::Module has been closed");
	return pOutput;
}

wasm::SinkInterface* writer::binary::Module::sink(const wasm::Function& function) {
	return 0;
}
void writer::binary::Module::close(const wasm::Module& module) {
	/* write the magic and version out */
	binary::Write<uint8_t>(pOutput, { 0x00, 0x61, 0x73, 0x6d, 0x10, 0x00, 0x00, 0x00 });

	/* write all sections out in order */
	fWriteSection(pPrototype, 0x01);
	fWriteSection(pImport, 0x02);
	fWriteSection(pFunction, 0x03);
	fWriteSection(pTable, 0x04);
	fWriteSection(pMemory, 0x05);
	fWriteSection(pGlobal, 0x06);
	fWriteSection(pExport, 0x07);

	/* write all code-sections out (all sinks will already have been closed by the wasm-framework) */
	fWriteSection(Section{ {}, uint32_t(pCode.size()) }, 0x0a);
	for (size_t i = 0; i < pCode.size(); ++i) {
		/* write the size of the code-size out and the locals/code out */
		binary::Write<uint32_t>(pOutput, { uint32_t(pCode[i].buffer.size() - pCode[i].localBytes) });
		pOutput.insert(pOutput.end(), pCode[i].buffer.begin(), pCode[i].buffer.end());
	}
}
void writer::binary::Module::addPrototype(const wasm::Prototype& prototype) {
	const std::vector<wasm::Param>& params = prototype.parameter();
	const std::vector<wasm::Type>& results = prototype.result();

	/* write the prototype out */
	binary::Write<uint8_t>(pPrototype.buffer, { 0x60 });
	++pPrototype.count;

	/* write the parameter out */
	binary::Write<uint32_t>(pPrototype.buffer, { uint32_t(params.size()) });
	for (size_t i = 0; i < params.size(); ++i)
		binary::Write<uint8_t>(pPrototype.buffer, { binary::GetType(params[i].type) });

	/* write the result out */
	binary::Write<uint32_t>(pPrototype.buffer, { uint32_t(results.size()) });
	for (size_t i = 0; i < results.size(); ++i)
		binary::Write<uint8_t>(pPrototype.buffer, { binary::GetType(results[i]) });
}
void writer::binary::Module::addMemory(const wasm::Memory& memory) {
	/* check if an export can be written out */
	if (memory.exported().valid()) {
		fWriteExport(memory.exported(), 0x02);
		binary::Write<uint32_t>(pExport.buffer, { memory.index() });
	}

	/* check if this is an import or setup the memory-entry */
	std::vector<uint8_t>& buffer = (memory.imported().valid() ? pImport : pMemory).buffer;
	if (memory.imported().valid())
		fWriteImport(memory.imported(), 0x02);
	else
		++pMemory.count;

	/* write the memory type out */
	binary::WriteLimit(buffer, memory.limit());
}
void writer::binary::Module::addTable(const wasm::Table& table) {
	/* check if an export can be written out */
	if (table.exported().valid()) {
		fWriteExport(table.exported(), 0x01);
		binary::Write<uint32_t>(pExport.buffer, { table.index() });
	}

	/* check if this is an import or setup the table-entry */
	std::vector<uint8_t>& buffer = (table.imported().valid() ? pImport : pTable).buffer;
	if (table.imported().valid())
		fWriteImport(table.imported(), 0x01);
	else
		++pTable.count;

	/* write the table type out */
	binary::Write<uint8_t>(buffer, { uint8_t(table.functions() ? 0x70 : 0x6f) });
	binary::WriteLimit(buffer, table.limit());
}
void writer::binary::Module::addGlobal(const wasm::Global& global) {
	/* check if an export can be written out */
	if (global.exported().valid()) {
		fWriteExport(global.exported(), 0x03);
		binary::Write<uint32_t>(pExport.buffer, { global.index() });
	}

	/* check if this is an import or setup the global */
	std::vector<uint8_t>& buffer = (global.imported().valid() ? pImport : pGlobal).buffer;
	if (global.imported().valid())
		fWriteImport(global.imported(), 0x03);
	else
		++pGlobal.count;

	/* write the global out and check if the actual value needs to be written out as well */
	binary::Write<uint8_t>(buffer, { binary::GetType(global.type()), uint8_t(global.mutating() ? 0x01 : 0x00) });
	if (global.imported().valid())
		return;

	/* write the initial value out */
	switch (global.type()) {
	case wasm::Type::i32:
		binary::Write<uint8_t>(pGlobal.buffer, { 0x41 });
		binary::Write<uint32_t>(pGlobal.buffer, { 0 });
		break;
	case wasm::Type::i64:
		binary::Write<uint8_t>(pGlobal.buffer, { 0x42 });
		binary::Write<uint64_t>(pGlobal.buffer, { 0 });
		break;
	case wasm::Type::f32:
		binary::Write<uint8_t>(pGlobal.buffer, { 0x43 });
		binary::Write<float>(pGlobal.buffer, { 0.0f });
		break;
	case wasm::Type::f64:
		binary::Write<uint8_t>(pGlobal.buffer, { 0x44 });
		binary::Write<double>(pGlobal.buffer, { 0.0 });
		break;
	case wasm::Type::refFunction:
		binary::Write<uint8_t>(pGlobal.buffer, { 0xd0, 0x70 });
		break;
	case wasm::Type::refExtern:
		binary::Write<uint8_t>(pGlobal.buffer, { 0xd0, 0x6f });
		break;
	}
	binary::Write<uint8_t>(pGlobal.buffer, { 0xb0 });
}
void writer::binary::Module::addFunction(const wasm::Function& function) {
	/* check if an export can be written out */
	if (function.exported().valid()) {
		fWriteExport(function.exported(), 0x00);
		binary::Write<uint32_t>(pExport.buffer, { function.index() });
	}

	/* check if this is an import or setup the function-entry and allocate the code-entry */
	std::vector<uint8_t>& buffer = (function.imported().valid() ? pImport : pFunction).buffer;
	if (function.imported().valid())
		fWriteImport(function.imported(), 0x00);
	else {
		pCode.emplace_back();
		++pFunction.count;
	}

	/* write the function type out */
	binary::Write<uint32_t>(buffer, { function.prototype().index() });
}
