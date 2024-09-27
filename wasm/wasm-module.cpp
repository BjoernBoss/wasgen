#include "wasm-module.h"
#include "wasm-sink.h"

wasm::Module::Module(wasm::ModuleInterface* interface) : pInterface{ interface } {}
wasm::Module::~Module() {
	fClose();
}

void wasm::Module::fClose() {
	if (pClosed)
		return;
	pClosed = true;

	/* close all remaining sinks */
	for (size_t i = 0; i < pFunction.list.size(); ++i) {
		if (pFunction.list[i].sink != 0)
			pFunction.list[i].sink->fClose();
	}

	/* mark the module as closed */
	pInterface->close(*this);
}
void wasm::Module::fCheckClosed() const {
	if (pClosed)
		util::fail(u8"Cannot change the closed module");
}

wasm::Prototype wasm::Module::prototype(std::initializer_list<wasm::Param> params, std::initializer_list<wasm::Type> result, std::u8string_view id) {
	fCheckClosed();

	/* validate the id and the parameter */
	std::u8string _id{ id };
	if (!_id.empty() && pPrototype.ids.contains(_id))
		util::fail(u8"Prototype with id [", _id, u8"] already defined");
	std::unordered_set<std::u8string> names;
	for (const auto& param : params) {
		if (param.id.empty())
			continue;
		if (names.contains(param.id))
			util::fail(u8"Parameter with name [", param.id, u8"] of prototype with id [", _id, u8"] already defined");
		names.insert(param.id);
	}

	/* setup the prototype-state */
	detail::PrototypeState state;
	state.parameter.insert(state.parameter.end(), params);
	state.result.insert(state.result.end(), result);

	/* allocate the next id and register the next prototype */
	if (!_id.empty())
		state.id = *pPrototype.ids.insert(_id).first;
	pPrototype.list.push_back(std::move(state));
	wasm::Prototype prototype{ *this, uint32_t(pPrototype.list.size() - 1) };

	/* notify the interface about the added prototype */
	pInterface->addPrototype(prototype);
	return prototype;
}
wasm::Memory wasm::Module::memory(const wasm::Limit& limit, std::u8string_view id, const wasm::Import& imported, const wasm::Export& exported) {
	fCheckClosed();

	/* validate the id */
	std::u8string _id{ id };
	if (!_id.empty() && pMemory.ids.contains(_id))
		util::fail(u8"Memory with id [", _id, u8"] already defined");

	/* setup the memory-state */
	detail::MemoryState state = { imported, exported, limit, {} };

	/* allocate the next id and register the next memory */
	if (!_id.empty())
		state.id = *pMemory.ids.insert(_id).first;
	pMemory.list.push_back(std::move(state));
	wasm::Memory memory{ *this, uint32_t(pMemory.list.size() - 1) };

	/* notify the interface about the added memory */
	pInterface->addMemory(memory);
	return memory;
}
wasm::Table wasm::Module::table(bool functions, const wasm::Limit& limit, std::u8string_view id, const wasm::Import& imported, const wasm::Export& exported) {
	fCheckClosed();

	/* validate the id */
	std::u8string _id{ id };
	if (!_id.empty() && pTable.ids.contains(_id))
		util::fail(u8"Table with id [", _id, u8"] already defined");

	/* setup the table-state */
	detail::TableState state = { imported, exported, limit, {}, functions };

	/* allocate the next id and register the next table */
	if (!_id.empty())
		state.id = *pTable.ids.insert(_id).first;
	pTable.list.push_back(std::move(state));
	wasm::Table table{ *this, uint32_t(pTable.list.size() - 1) };

	/* notify the interface about the added table */
	pInterface->addTable(table);
	return table;
}
wasm::Global wasm::Module::global(wasm::Type type, bool mutating, std::u8string_view id, const wasm::Import& imported, const wasm::Export& exported) {
	fCheckClosed();

	/* validate the id */
	std::u8string _id{ id };
	if (!_id.empty() && pGlobal.ids.contains(_id))
		util::fail(u8"Global with id [", _id, u8"] already defined");

	/* setup the global */
	detail::GlobalState state = { imported, exported, {}, type, mutating };

	/* allocate the next id and register the next global */
	if (!_id.empty())
		state.id = *pGlobal.ids.insert(_id).first;
	pGlobal.list.push_back(std::move(state));
	wasm::Global global{ *this, uint32_t(pGlobal.list.size() - 1) };

	/* notify the interface about the added global */
	pInterface->addGlobal(global);
	return global;
}
wasm::Function wasm::Module::function(const wasm::Prototype& prototype, std::u8string_view id, const wasm::Import& imported, const wasm::Export& exported) {
	fCheckClosed();

	/* validate the id and the prototype */
	std::u8string _id{ id };
	if (!_id.empty() && pFunction.ids.contains(_id))
		util::fail(u8"Function with id [", _id, u8"] already defined");
	if (prototype.valid() && &prototype.module() != this)
		util::fail(u8"Prototype for function with id [", _id, u8"] must originate from the same module");

	/* setup the function */
	detail::FunctionState state = { imported, exported, {}, prototype };

	/* allocate the next id and register the next function */
	if (!_id.empty())
		state.id = *pFunction.ids.insert(_id).first;
	pFunction.list.push_back(std::move(state));
	wasm::Function function{ *this, uint32_t(pFunction.list.size() - 1) };

	/* notify the interface about the added function */
	pInterface->addFunction(function);
	return function;
}
void wasm::Module::close() {
	fClose();
}

wasm::List<wasm::Prototype, wasm::Module::PrototypeList> wasm::Module::prototypes() const {
	return { Module::PrototypeList{ const_cast<wasm::Module*>(this) } };
}
wasm::List<wasm::Memory, wasm::Module::MemoryList> wasm::Module::memories() const {
	return { Module::MemoryList{ const_cast<wasm::Module*>(this) } };
}
wasm::List<wasm::Table, wasm::Module::TableList> wasm::Module::tables() const {
	return { Module::TableList{ const_cast<wasm::Module*>(this) } };
}
wasm::List<wasm::Global, wasm::Module::GlobalList> wasm::Module::globals() const {
	return { Module::GlobalList{ const_cast<wasm::Module*>(this) } };
}
wasm::List<wasm::Function, wasm::Module::FunctionList> wasm::Module::functions() const {
	return { Module::FunctionList{ const_cast<wasm::Module*>(this) } };
}
