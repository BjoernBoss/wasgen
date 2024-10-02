#include "binary-module.h"
#include "binary-sink.h"

void writer::binary::Module::fWriteImport(const wasm::Import& imported, uint8_t type) {
	binary::WriteString(pImport.buffer, imported.module);
	binary::WriteString(pImport.buffer, imported.name);
	pImport.buffer.push_back(type);
	++pImport.count;
}
void writer::binary::Module::fWriteExport(const wasm::Export& exported, uint8_t type) {
	binary::WriteString(pExport.buffer, exported.name);
	pExport.buffer.push_back(type);
	++pExport.count;
}
void writer::binary::Module::fWriteSection(const Section& section, uint32_t size, uint8_t id) {
	if (section.count == 0)
		return;

	/* write the id out */
	pOutput.push_back(id);

	/* write the byte-size and count out */
	binary::WriteUInt(pOutput, uint64_t(size + binary::CountUInt(section.count)));
	binary::WriteUInt(pOutput, section.count);

	/* write the actual data out */
	pOutput.insert(pOutput.end(), section.buffer.begin(), section.buffer.end());
}

const std::vector<uint8_t>& writer::binary::Module::output() const {
	if (pOutput.empty())
		util::fail(u8"Cannot produce binary-writer module output before the wrapping wasm::Module has been closed");
	return pOutput;
}

wasm::SinkInterface* writer::binary::Module::sink(const wasm::Function& function) {
	return new binary::Sink{ this, uint32_t(function.index() - pCodeOffset) };
}
void writer::binary::Module::close(const wasm::Module& module) {
	/* write the magic and version out */
	binary::WriteBytes(pOutput, { 0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00 });

	/* write all sections out in order */
	fWriteSection(pPrototype, uint32_t(pPrototype.buffer.size()), 0x01);
	fWriteSection(pImport, uint32_t(pImport.buffer.size()), 0x02);
	fWriteSection(pFunction, uint32_t(pFunction.buffer.size()), 0x03);
	fWriteSection(pTable, uint32_t(pTable.buffer.size()), 0x04);
	fWriteSection(pMemory, uint32_t(pMemory.buffer.size()), 0x05);
	fWriteSection(pGlobal, uint32_t(pGlobal.buffer.size()), 0x06);
	fWriteSection(pExport, uint32_t(pExport.buffer.size()), 0x07);

	/* compute the byte-size of the code-section and close all functions (all sinks will already have been closed by the wasm-framework) */
	uint32_t codeSize = 0;
	for (size_t i = 0; i < pCode.size(); ++i) {
		/* check if the code-section needs to be closed properly */
		if (pCode[i].empty())
			pCode[i].push_back(0x0b);

		/* compute the estimated size */
		codeSize += uint32_t(pCode[i].size()) + binary::CountUInt(pCode[i].size());
	}

	/* write the code-section out */
	fWriteSection(Section{ {}, uint32_t(pCode.size()) }, codeSize, 0x0a);
	for (size_t i = 0; i < pCode.size(); ++i) {
		/* write the size of the body out and the body itself */
		binary::WriteUInt(pOutput, pCode[i].size());
		pOutput.insert(pOutput.end(), pCode[i].begin(), pCode[i].end());
	}
}
void writer::binary::Module::addPrototype(const wasm::Prototype& prototype) {
	const std::vector<wasm::Param>& params = prototype.parameter();
	const std::vector<wasm::Type>& results = prototype.result();

	/* write the prototype out */
	pPrototype.buffer.push_back(0x60);
	++pPrototype.count;

	/* write the parameter out */
	binary::WriteUInt(pPrototype.buffer, uint32_t(params.size()));
	for (size_t i = 0; i < params.size(); ++i)
		pPrototype.buffer.push_back(binary::GetType(params[i].type));

	/* write the result out */
	binary::WriteUInt(pPrototype.buffer, uint32_t(results.size()));
	for (size_t i = 0; i < results.size(); ++i)
		pPrototype.buffer.push_back(binary::GetType(results[i]));
}
void writer::binary::Module::addMemory(const wasm::Memory& memory) {
	/* check if an export can be written out */
	if (memory.exported().valid()) {
		fWriteExport(memory.exported(), 0x02);
		binary::WriteUInt(pExport.buffer, memory.index());
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
		binary::WriteUInt(pExport.buffer, table.index());
	}

	/* check if this is an import or setup the table-entry */
	std::vector<uint8_t>& buffer = (table.imported().valid() ? pImport : pTable).buffer;
	if (table.imported().valid())
		fWriteImport(table.imported(), 0x01);
	else
		++pTable.count;

	/* write the table type out */
	buffer.push_back(table.functions() ? 0x70 : 0x6f);
	binary::WriteLimit(buffer, table.limit());
}
void writer::binary::Module::addGlobal(const wasm::Global& global) {
	/* check if an export can be written out */
	if (global.exported().valid()) {
		fWriteExport(global.exported(), 0x03);
		binary::WriteUInt(pExport.buffer, global.index());
	}

	/* check if this is an import or setup the global */
	std::vector<uint8_t>& buffer = (global.imported().valid() ? pImport : pGlobal).buffer;
	if (global.imported().valid())
		fWriteImport(global.imported(), 0x03);
	else
		++pGlobal.count;

	/* write the global out and check if the actual value needs to be written out as well */
	binary::WriteBytes(buffer, { binary::GetType(global.type()), uint8_t(global.mutating() ? 0x01 : 0x00) });
	if (global.imported().valid())
		return;

	/* write the initial value out */
	switch (global.type()) {
	case wasm::Type::i32:
		pGlobal.buffer.push_back(0x41);
		binary::WriteUInt(pGlobal.buffer, 0);
		break;
	case wasm::Type::i64:
		pGlobal.buffer.push_back(0x42);
		binary::WriteSInt(pGlobal.buffer, 0);
		break;
	case wasm::Type::f32:
		pGlobal.buffer.push_back(0x43);
		binary::WriteFloat(pGlobal.buffer, 0.0f);
		break;
	case wasm::Type::f64:
		pGlobal.buffer.push_back(0x44);
		binary::WriteDouble(pGlobal.buffer, 0.0);
		break;
	case wasm::Type::refFunction:
		binary::WriteBytes(pGlobal.buffer, { 0xd0, 0x70 });
		break;
	case wasm::Type::refExtern:
		binary::WriteBytes(pGlobal.buffer, { 0xd0, 0x6f });
		break;
	}
	pGlobal.buffer.push_back(0xb0);
}
void writer::binary::Module::addFunction(const wasm::Function& function) {
	/* check if an export can be written out */
	if (function.exported().valid()) {
		fWriteExport(function.exported(), 0x00);
		binary::WriteUInt(pExport.buffer, function.index());
	}

	/* check if this is an import or setup the function-entry and allocate the code-entry */
	std::vector<uint8_t>& buffer = (function.imported().valid() ? pImport : pFunction).buffer;
	if (function.imported().valid())
		fWriteImport(function.imported(), 0x00);
	else {
		if (pCode.empty())
			pCodeOffset = function.index();
		pCode.emplace_back();
		++pFunction.count;
	}

	/* write the function type out */
	binary::WriteUInt(buffer, function.prototype().index());
}
