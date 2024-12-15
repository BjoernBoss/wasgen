/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2024 Bjoern Boss Henrichsen */
#include "binary-module.h"
#include "binary-sink.h"

void wasm::binary::Module::fWriteImport(const std::u8string& importModule, std::u8string_view id, uint8_t type) {
	binary::WriteString(pImport.buffer, importModule);
	binary::WriteString(pImport.buffer, id);
	pImport.buffer.push_back(type);
	++pImport.count;
}
void wasm::binary::Module::fWriteExport(std::u8string_view id, uint8_t type) {
	binary::WriteString(pExport.buffer, id);
	pExport.buffer.push_back(type);
	++pExport.count;
}
void wasm::binary::Module::fWriteSection(const Section& section, bool placeCount, uint8_t id) {
	if (section.count == 0)
		return;

	/* write the id out */
	pOutput.push_back(id);

	/* write the byte-size and count out */
	binary::WriteUInt(pOutput, uint64_t(section.buffer.size() + (placeCount ? binary::CountUInt(section.count) : 0)));
	if (placeCount)
		binary::WriteUInt(pOutput, section.count);

	/* write the actual data out */
	pOutput.insert(pOutput.end(), section.buffer.begin(), section.buffer.end());
}
void wasm::binary::Module::fWriteSection(const Deferred& section, bool placeSlotSize, uint8_t id) {
	if (section.data.empty())
		return;

	/* write the id out */
	pOutput.push_back(id);

	/* compute the overall size */
	uint32_t size = 0;
	for (size_t i = 0; i < section.data.size(); ++i)
		size += uint32_t(section.data[i].size()) + (placeSlotSize ? binary::CountUInt(section.data[i].size()) : 0);

	/* write the byte-size and count out */
	binary::WriteUInt(pOutput, uint64_t(size + binary::CountUInt(section.data.size())));
	binary::WriteUInt(pOutput, section.data.size());

	/* write the actual data out */
	for (size_t i = 0; i < section.data.size(); ++i) {
		if (placeSlotSize)
			binary::WriteUInt(pOutput, section.data[i].size());
		pOutput.insert(pOutput.end(), section.data[i].begin(), section.data[i].end());
	}
}

const std::vector<uint8_t>& wasm::binary::Module::output() const {
	if (pOutput.empty())
		throw wasm::Exception{ L"Cannot produce binary-writer module output before the wrapping wasm::Module has been closed" };
	return pOutput;
}

wasm::SinkInterface* wasm::binary::Module::sink(const wasm::Function& function) {
	return new binary::Sink{ this, uint32_t(function.index() - pCode.indexOffset) };
}
void wasm::binary::Module::close(const wasm::Module& module) {
	/* all globals will have been set and all functions will have been sunken and flushed by the wasm-framework */

	/* write the magic and version out */
	binary::WriteBytes(pOutput, { 0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00 });

	/* write all sections out in order */
	fWriteSection(pPrototype, true, 0x01);
	fWriteSection(pImport, true, 0x02);
	fWriteSection(pFunction, true, 0x03);
	fWriteSection(pTable, false, 0x04);
	fWriteSection(pMemory, false, 0x05);
	fWriteSection(pGlobal, false, 0x06);
	fWriteSection(pExport, true, 0x07);
	fWriteSection(pStart, false, 0x08);
	fWriteSection(pElement, true, 0x09);
	fWriteSection(pCode, true, 0x0a);
	fWriteSection(pData, true, 0x0b);
}
void wasm::binary::Module::addPrototype(const wasm::Prototype& prototype) {
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
void wasm::binary::Module::addMemory(const wasm::Memory& memory) {
	/* check if an export can be written out */
	if (memory.exported()) {
		fWriteExport(memory.id(), 0x02);
		binary::WriteUInt(pExport.buffer, memory.index());
	}

	/* check if this is an import and write it out (imports will immediately have a valid limit) */
	if (memory.imported()) {
		fWriteImport(memory.importModule(), memory.id(), 0x02);
		binary::WriteLimit(pImport.buffer, memory.limit());
	}
	else {
		/* setup the memory-entry and check if the limit can already be written out */
		if (pMemory.data.empty())
			pMemory.indexOffset = memory.index();
		pMemory.data.emplace_back();
		if (memory.limit().valid())
			binary::WriteLimit(pMemory.data.back(), memory.limit());
	}
}
void wasm::binary::Module::addTable(const wasm::Table& table) {
	/* check if an export can be written out */
	if (table.exported()) {
		fWriteExport(table.id(), 0x01);
		binary::WriteUInt(pExport.buffer, table.index());
	}

	/* check if this is an import and write it out (imports will immediately have a valid limit) */
	if (table.imported()) {
		fWriteImport(table.importModule(), table.id(), 0x01);
	}
	else {
		/* allocate the next table entry */
		if (pTable.data.empty())
			pTable.indexOffset = table.index();
		pTable.data.emplace_back();
	}
	std::vector<uint8_t>& buffer = (table.imported() ? pImport.buffer : pTable.data.back());

	/* write the table-type out and check if the limit can already be written out (will immediately be valid for imports) */
	buffer.push_back(table.functions() ? 0x70 : 0x6f);
	if (table.limit().valid())
		binary::WriteLimit(buffer, table.limit());
}
void wasm::binary::Module::addGlobal(const wasm::Global& global) {
	/* check if an export can be written out */
	if (global.exported()) {
		fWriteExport(global.id(), 0x03);
		binary::WriteUInt(pExport.buffer, global.index());
	}

	/* check if this is an import or allocate the global entry */
	if (global.imported())
		fWriteImport(global.importModule(), global.id(), 0x03);
	else {
		/* allocate the next global entry */
		if (pGlobal.data.empty())
			pGlobal.indexOffset = global.index();
		pGlobal.data.emplace_back();
	}
	std::vector<uint8_t>& buffer = (global.imported() ? pImport.buffer : pGlobal.data.back());

	/* write the global-header out */
	binary::WriteBytes(buffer, { binary::GetType(global.type()), uint8_t(global.mutating() ? 0x01 : 0x00) });
}
void wasm::binary::Module::addFunction(const wasm::Function& function) {
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
		/* allocate the next code entry */
		if (pCode.data.empty())
			pCode.indexOffset = function.index();
		pCode.data.emplace_back();
		++pFunction.count;
	}

	/* write the function type out */
	binary::WriteUInt(buffer, function.prototype().index());
}
void wasm::binary::Module::setMemoryLimit(const wasm::Memory& memory) {
	binary::WriteLimit(pMemory.data[size_t(memory.index() - pMemory.indexOffset)], memory.limit());
}
void wasm::binary::Module::setTableLimit(const wasm::Table& table) {
	binary::WriteLimit(pTable.data[size_t(table.index() - pTable.indexOffset)], table.limit());
}
void wasm::binary::Module::setStartup(const wasm::Function& function) {
	++pStart.count;
	binary::WriteUInt(pStart.buffer, function.index());
}
void wasm::binary::Module::setValue(const wasm::Global& global, const wasm::Value& value) {
	wasm::binary::WriteValue(pGlobal.data[size_t(global.index() - pGlobal.indexOffset)], value);
}
void wasm::binary::Module::writeData(const wasm::Memory& memory, const wasm::Value& offset, const uint8_t* data, uint32_t count) {
	/* setup the next data entry */
	++pData.count;
	pData.buffer.push_back(0x02);

	/* write the memory-index and offset out */
	binary::WriteUInt(pData.buffer, memory.index());
	binary::WriteValue(pData.buffer, offset);

	/* write the data-vector out */
	binary::WriteUInt(pData.buffer, count);
	pData.buffer.insert(pData.buffer.end(), data, data + count);
}
void wasm::binary::Module::writeElements(const wasm::Table& table, const wasm::Value& offset, const wasm::Value* values, uint32_t count) {
	/* check if the entire list of values consists of functions */
	bool allFunctions = std::all_of(values, values + count,
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
	binary::WriteUInt(pElement.buffer, count);
	if (allFunctions) {
		for (uint32_t i = 0; i < count; ++i)
			binary::WriteUInt(pElement.buffer, values[i].function().index());
	}
	else for (uint32_t i = 0; i < count; ++i)
		binary::WriteValue(pElement.buffer, values[i]);
}
