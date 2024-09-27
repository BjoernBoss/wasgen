#include "text-module.h"
#include "text-sink.h"

const std::u8string& writer::text::Module::output() const {
	if (pOutput.empty())
		util::fail(u8"Cannot produce text-writer module output before the wrapping wasm::Module has been closed");
	return pOutput;
}

wasm::SinkInterface* writer::text::Module::sink(const wasm::Function& function) {
	std::u8string header;
	std::swap(header, pFunctions[function.index()]);

	/* allocate the new sink for the function */
	return new text::Sink{ this, std::move(header) };
}
void writer::text::Module::close(const wasm::Module& module) {
	/* flush all remaining functions to the body (all sinks will already have been closed by the wasm-framework) */
	for (size_t i = 0; i < pFunctions.size(); ++i) {
		if (!pFunctions[i].empty())
			str::BuildTo(pDefined, pFunctions[i], u8')');
		pFunctions[i].clear();
	}

	/* merge the remaining content together to construct the complete module-text */
	if (pImports.empty() && pDefined.empty())
		pOutput = u8"(module)";
	else
		str::BuildTo(pOutput, u8"(module", pImports, pDefined, u8"\n)");
}
void writer::text::Module::addPrototype(const wasm::Prototype& prototype) {
	const std::vector<wasm::Param>& params = prototype.parameter();
	const std::vector<wasm::Type>& results = prototype.result();

	/* write the actual type to the definition-body */
	str::BuildTo(pDefined, u8"\n  (type", text::MakeId(prototype.id()), u8" (func");
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
	const wasm::Import& imp = memory.imported();

	str::BuildTo((imp.valid() ? pImports : pDefined),
		u8"\n  (memory",
		text::MakeId(memory.id()),
		text::MakeExport(memory.exported()),
		text::MakeImport(imp),
		text::MakeLimit(memory.limit()),
		u8')');
}
void writer::text::Module::addTable(const wasm::Table& table) {
	const wasm::Import& imp = table.imported();

	str::BuildTo((imp.valid() ? pImports : pDefined),
		u8"\n  (table",
		text::MakeId(table.id()),
		text::MakeExport(table.exported()),
		text::MakeImport(imp),
		text::MakeLimit(table.limit()),
		table.functions() ? u8" funcref)" : u8" externref)");
}
void writer::text::Module::addGlobal(const wasm::Global& global) {
	/* construct the type-description */
	std::u8string typeString = (global.mutating() ?
		str::Build<std::u8string>(u8" (mut", text::MakeType(global.type()), u8')') :
		std::u8string(text::MakeType(global.type())));

	/* construct the actual global-definition and write it out to the corresponding body */
	const wasm::Import& imp = global.imported();
	str::BuildTo((imp.valid() ? pImports : pDefined),
		u8"\n  (global",
		text::MakeId(global.id()),
		text::MakeExport(global.exported()),
		text::MakeImport(imp),
		typeString,
		u8')');
}
void writer::text::Module::addFunction(const wasm::Function& function) {
	std::u8string funcText;

	/* construct the function-header text */
	const wasm::Import& imp = function.imported();
	str::BuildTo(funcText, u8"\n  (func",
		text::MakeId(function.id()),
		text::MakeExport(function.exported()),
		text::MakeImport(imp),
		text::MakePrototype(function.prototype())
	);

	/* check if this is an import, in which case it can be produced immediately, otherwise a proper sink needs to be set-up */
	if (imp.valid()) {
		pFunctions.emplace_back();
		pImports.append(funcText).append(1, u8')');
	}
	else
		pFunctions.push_back(std::move(funcText));
}
