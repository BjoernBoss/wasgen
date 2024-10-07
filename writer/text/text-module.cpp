/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2024 Bjoern Boss Henrichsen */
#include "text-module.h"
#include "text-sink.h"

writer::text::Module::Module(std::u8string_view indent) : pIndent{ indent } {}

const std::u8string& writer::text::Module::output() const {
	if (pOutput.empty())
		throw wasm::Exception{ L"Cannot produce text-writer module output before the wrapping wasm::Module has been closed" };
	return pOutput;
}

wasm::SinkInterface* writer::text::Module::sink(const wasm::Function& function) {
	std::u8string header;
	std::swap(header, pFunctions[function.index()]);

	/* allocate the new sink for the function */
	return new text::Sink{ this, std::move(header) };
}
void writer::text::Module::close(const wasm::Module& module) {
	/* merge the remaining content together to construct the complete module-text (all globals will
	*	have been set and all functions will have been sunken and flushed by the wasm-framework) */
	if (pImports.empty() && pDefined.empty())
		pOutput = u8"(module)";
	else
		str::BuildTo(pOutput, u8"(module", pImports, pDefined, u8"\n)");
}
void writer::text::Module::addPrototype(const wasm::Prototype& prototype) {
	const std::vector<wasm::Param>& params = prototype.parameter();
	const std::vector<wasm::Type>& results = prototype.result();

	/* write the actual type to the definition-body */
	str::BuildTo(pDefined, u8'\n', pIndent, u8"(type", text::MakeId(prototype.id()), u8" (func");
	for (size_t i = 0; i < params.size(); ++i)
		str::BuildTo(pDefined, u8" (param", text::MakeId(params[i].id), text::MakeType(params[i].type), u8')');
	if (!results.empty()) {
		pDefined.append(u8" (result");
		for (size_t i = 0; i < results.size(); ++i)
			str::BuildTo(pDefined, text::MakeType(results[i]));
		pDefined.push_back(u8')');
	}
	pDefined.append(u8"))");
}
void writer::text::Module::addMemory(const wasm::Memory& memory) {
	str::BuildTo((memory.imported() ? pImports : pDefined),
		u8'\n', pIndent, u8"(memory",
		text::MakeId(memory.id()),
		text::MakeExport(memory.exported(), memory.id()),
		text::MakeImport(memory.importModule(), memory.id()),
		text::MakeLimit(memory.limit()),
		u8')');
}
void writer::text::Module::addTable(const wasm::Table& table) {
	str::BuildTo((table.imported() ? pImports : pDefined),
		u8'\n', pIndent, u8"(table",
		text::MakeId(table.id()),
		text::MakeExport(table.exported(), table.id()),
		text::MakeImport(table.importModule(), table.id()),
		text::MakeLimit(table.limit()),
		table.functions() ? u8" funcref)" : u8" externref)");
}
void writer::text::Module::addGlobal(const wasm::Global& global) {
	std::u8string globText;

	/* construct the type-description */
	std::u8string typeString = (global.mutating() ?
		str::Build<std::u8string>(u8" (mut", text::MakeType(global.type()), u8')') :
		std::u8string(text::MakeType(global.type())));

	/* construct the global text */
	str::BuildTo(globText,
		u8'\n', pIndent, u8"(global",
		text::MakeId(global.id()),
		text::MakeExport(global.exported(), global.id()),
		text::MakeImport(global.importModule(), global.id()),
		typeString);

	/* check if this is an import, in which case it can be produced immediately, otherwise a value will be written later */
	if (global.imported()) {
		pGlobals.emplace_back();
		pImports.append(globText).append(1, u8')');
	}
	else
		pGlobals.push_back(std::move(globText));
}
void writer::text::Module::addFunction(const wasm::Function& function) {
	std::u8string funcText;

	/* construct the function-header text */
	str::BuildTo(funcText, u8'\n', pIndent, u8"(func",
		text::MakeId(function.id()),
		text::MakeExport(function.exported(), function.id()),
		text::MakeImport(function.importModule(), function.id()),
		text::MakePrototype(function.prototype())
	);

	/* check if this is an import, in which case it can be produced immediately, otherwise a proper sink needs to be set-up */
	if (function.imported()) {
		pFunctions.emplace_back();
		pImports.append(funcText).append(1, u8')');
	}
	else
		pFunctions.push_back(std::move(funcText));
}
void writer::text::Module::setValue(const wasm::Global& global, const wasm::Value& value) {
	/* write the value to the global and flush it out */
	str::BuildTo(pDefined,
		pGlobals[global.index()],
		u8' ',
		text::MakeValue(value), u8')');
	pGlobals[global.index()].clear();
}
void writer::text::Module::writeData(const wasm::Memory& memory, const wasm::Value& offset, const std::vector<uint8_t>& data) {
	std::u8string dataText;

	/* construct the data-string */
	for (size_t i = 0; i < data.size(); ++i) {
		switch (char8_t(data[i])) {
		case u8'\t':
			dataText.append(u8"\\t");
			break;
		case u8'\n':
			dataText.append(u8"\\n");
			break;
		case u8'\r':
			dataText.append(u8"\\r");
			break;
		case u8'\"':
			dataText.append(u8"\\\"");
			break;
		case u8'\\':
			dataText.append(u8"\\\\");
			break;
		default:
			if (cp::prop::IsAscii(char32_t(data[i])) && !cp::prop::IsControl(char32_t(data[i])))
				dataText.push_back(char8_t(data[i]));
			else
				str::FormatTo(dataText, u8"\\{:02x}", data[i]);
			break;
		}
	}

	/* write the data-definition out */
	str::BuildTo(pDefined,
		u8'\n', pIndent, u8"(data (memory ",
		memory.toString(),
		u8") (offset ",
		text::MakeValue(offset),
		u8") \"",
		dataText,
		u8"\")");
}
void writer::text::Module::writeElements(const wasm::Table& table, const wasm::Value& offset, const std::vector<wasm::Value>& values) {
	/* write the elements header out */
	str::BuildTo(pDefined,
		u8'\n', pIndent, u8"(elem (table ",
		table.toString(),
		u8") (offset ",
		text::MakeValue(offset),
		(table.functions() ? u8") funcref" : u8") externref"));

	/* write the items out and close the element list */
	for (size_t i = 0; i < values.size(); ++i)
		str::BuildTo(pDefined, u8" (item ", text::MakeValue(values[i]), u8')');
	pDefined.push_back(u8')');
}
