#include "binary-module.h"
#include "binary-sink.h"

void writer::binary::Module::fWriteImport(const std::u8string& importModule, std::u8string_view id, uint8_t type) {
	binary::WriteString(pImport.buffer, importModule);
	binary::WriteString(pImport.buffer, id);
	pImport.buffer.push_back(type);
	++pImport.count;
}
void writer::binary::Module::fWriteExport(std::u8string_view id, uint8_t type) {
	binary::WriteString(pExport.buffer, id);
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
	/* all globals will have been set and all functions will have been sunken and flushed by the wasm-framework */

	/* write the magic and version out */
	binary::WriteBytes(pOutput, { 0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00 });

	/* write all sections out in order */
	fWriteSection(pPrototype, uint32_t(pPrototype.buffer.size()), 0x01);
	fWriteSection(pImport, uint32_t(pImport.buffer.size()), 0x02);
	fWriteSection(pFunction, uint32_t(pFunction.buffer.size()), 0x03);
	fWriteSection(pTable, uint32_t(pTable.buffer.size()), 0x04);
	fWriteSection(pMemory, uint32_t(pMemory.buffer.size()), 0x05);

	/* write the global section out */
	uint32_t globSize = 0;
	for (size_t i = 0; i < pGlobal.size(); ++i)
		globSize += uint32_t(pGlobal[i].size());
	fWriteSection(Section{ {}, uint32_t(pGlobal.size()) }, globSize, 0x06);
	for (size_t i = 0; i < pGlobal.size(); ++i)
		pOutput.insert(pOutput.end(), pGlobal[i].begin(), pGlobal[i].end());

	/* write the intermediate sections out */
	fWriteSection(pExport, uint32_t(pExport.buffer.size()), 0x07);
	fWriteSection(pElement, uint32_t(pElement.buffer.size()), 0x09);

	/* write the code-section out */
	uint32_t codeSize = 0;
	for (size_t i = 0; i < pCode.size(); ++i)
		codeSize += uint32_t(pCode[i].size()) + binary::CountUInt(pCode[i].size());
	fWriteSection(Section{ {}, uint32_t(pCode.size()) }, codeSize, 0x0a);
	for (size_t i = 0; i < pCode.size(); ++i) {
		binary::WriteUInt(pOutput, pCode[i].size());
		pOutput.insert(pOutput.end(), pCode[i].begin(), pCode[i].end());
	}

	/* write the data-section out */
	fWriteSection(pData, uint32_t(pData.buffer.size()), 0x0b);
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
	if (memory.exported()) {
		fWriteExport(memory.id(), 0x02);
		binary::WriteUInt(pExport.buffer, memory.index());
	}

	/* check if this is an import or setup the memory-entry */
	std::vector<uint8_t>& buffer = (memory.imported() ? pImport : pMemory).buffer;
	if (memory.imported())
		fWriteImport(memory.importModule(), memory.id(), 0x02);
	else
		++pMemory.count;

	/* write the memory type out */
	binary::WriteLimit(buffer, memory.limit());
}
void writer::binary::Module::addTable(const wasm::Table& table) {
	/* check if an export can be written out */
	if (table.exported()) {
		fWriteExport(table.id(), 0x01);
		binary::WriteUInt(pExport.buffer, table.index());
	}

	/* check if this is an import or setup the table-entry */
	std::vector<uint8_t>& buffer = (table.imported() ? pImport : pTable).buffer;
	if (table.imported())
		fWriteImport(table.importModule(), table.id(), 0x01);
	else
		++pTable.count;

	/* write the table type out */
	buffer.push_back(table.functions() ? 0x70 : 0x6f);
	binary::WriteLimit(buffer, table.limit());
}
void writer::binary::Module::addGlobal(const wasm::Global& global) {
	/* check if an export can be written out */
	if (global.exported()) {
		fWriteExport(global.id(), 0x03);
		binary::WriteUInt(pExport.buffer, global.index());
	}

	/* check if this is an import or allocate the global entry */
	if (global.imported())
		fWriteImport(global.importModule(), global.id(), 0x03);
	else {
		if (pGlobal.empty())
			pGlobOffset = global.index();
		pGlobal.emplace_back();
	}
	std::vector<uint8_t>& buffer = (global.imported() ? pImport.buffer : pGlobal.back());

	/* write the global-header out */
	binary::WriteBytes(buffer, { binary::GetType(global.type()), uint8_t(global.mutating() ? 0x01 : 0x00) });
}
void writer::binary::Module::addFunction(const wasm::Function& function) {
	/* check if an export can be written out */
	if (function.exported()) {
		fWriteExport(function.id(), 0x00);
		binary::WriteUInt(pExport.buffer, function.index());
	}

	/* check if this is an import or setup the function-entry and allocate the code-entry */
	std::vector<uint8_t>& buffer = (function.imported() ? pImport : pFunction).buffer;
	if (function.imported())
		fWriteImport(function.importModule(), function.id(), 0x00);
	else {
		if (pCode.empty())
			pCodeOffset = function.index();
		pCode.emplace_back();
		++pFunction.count;
	}

	/* write the function type out */
	binary::WriteUInt(buffer, function.prototype().index());
}
void writer::binary::Module::setValue(const wasm::Global& global, const wasm::Value& value) {
	writer::binary::WriteValue(pGlobal[size_t(global.index() - pGlobOffset)], value);
}
void writer::binary::Module::writeData(const wasm::Memory& memory, const wasm::Value& offset, const std::vector<uint8_t>& data) {
	/* setup the next data entry */
	++pData.count;
	pData.buffer.push_back(0x02);

	/* write the memory-index and offset out */
	binary::WriteUInt(pData.buffer, memory.index());
	binary::WriteValue(pData.buffer, offset);

	/* write the data-vector out */
	binary::WriteUInt(pData.buffer, data.size());
	pData.buffer.insert(pData.buffer.end(), data.begin(), data.end());
}
void writer::binary::Module::writeElements(const wasm::Table& table, const wasm::Value& offset, const std::vector<wasm::Value>& values) {
	/* check if the entire list of values consists of functions */
	bool allFunctions = std::all_of(values.begin(), values.end(),
		[](const wasm::Value& value) { return (value.type() == wasm::ValType::refFunction && value.function().valid()); }
	);

	/* setup the next element entry */
	++pElement.count;
	pElement.buffer.push_back(allFunctions ? 0x02 : 0x06);

	/* write the table-index, offset, and ref-type out */
	binary::WriteUInt(pElement.buffer, table.index());
	binary::WriteValue(pElement.buffer, offset);
	if (allFunctions)
		pElement.buffer.push_back(0x00);
	else
		pElement.buffer.push_back(binary::GetType(table.functions() ? wasm::Type::refFunction : wasm::Type::refExtern));

	/* write the element-vector out */
	binary::WriteUInt(pElement.buffer, values.size());
	if (allFunctions) {
		for (size_t i = 0; i < values.size(); ++i)
			binary::WriteUInt(pElement.buffer, values[i].function().index());
	}
	else for (size_t i = 0; i < values.size(); ++i)
		binary::WriteValue(pElement.buffer, values[i]);
}
