#include "_wasm-module.h"
#include "_wasm-sink.h"

#include "../util/logging.h"

wasm::_Module::_Module(wasm::_ModuleInterface* interface) : pInterface{ interface } {}
wasm::_Module::~_Module() {
	fClose();
}

void wasm::_Module::fClose() {
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
void wasm::_Module::fCheckClosed() const {
	if (pClosed)
		util::fail(u8"Cannot change the closed module");
}

wasm::_Prototype wasm::_Module::prototype(std::initializer_list<wasm::_Param> params, std::initializer_list<wasm::_Type> result, std::u8string_view id) {
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
	wasm::_Prototype prototype{ *this, uint32_t(pPrototype.list.size() - 1) };

	/* notify the interface about the added prototype */
	pInterface->addPrototype(prototype);
	return prototype;
}
wasm::_Memory wasm::_Module::memory(const wasm::_Limit& limit, std::u8string_view id, const wasm::_Import& imported, const wasm::_Export& exported) {
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
	wasm::_Memory memory{ *this, uint32_t(pMemory.list.size() - 1) };

	/* notify the interface about the added memory */
	pInterface->addMemory(memory);
	return memory;
}
wasm::_Table wasm::_Module::table(bool functions, const wasm::_Limit& limit, std::u8string_view id, const wasm::_Import& imported, const wasm::_Export& exported) {
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
	wasm::_Table table{ *this, uint32_t(pTable.list.size() - 1) };

	/* notify the interface about the added table */
	pInterface->addTable(table);
	return table;
}
wasm::_Global wasm::_Module::global(wasm::_Type type, bool mutating, std::u8string_view id, const wasm::_Import& imported, const wasm::_Export& exported) {
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
	wasm::_Global global{ *this, uint32_t(pGlobal.list.size() - 1) };

	/* notify the interface about the added global */
	pInterface->addGlobal(global);
	return global;
}
wasm::_Function wasm::_Module::function(const wasm::_Prototype& prototype, std::u8string_view id, const wasm::_Import& imported, const wasm::_Export& exported) {
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
	wasm::_Function function{ *this, uint32_t(pFunction.list.size() - 1) };

	/* notify the interface about the added function */
	pInterface->addFunction(function);
	return function;
}
void wasm::_Module::close() {
	fClose();
}

wasm::_List<wasm::_Prototype, wasm::_Module::_PrototypeList> wasm::_Module::prototypes() const {
	return { _Module::_PrototypeList{ const_cast<wasm::_Module*>(this) } };
}
wasm::_List<wasm::_Memory, wasm::_Module::_MemoryList> wasm::_Module::memories() const {
	return { _Module::_MemoryList{ const_cast<wasm::_Module*>(this) } };
}
wasm::_List<wasm::_Table, wasm::_Module::_TableList> wasm::_Module::tables() const {
	return { _Module::_TableList{ const_cast<wasm::_Module*>(this) } };
}
wasm::_List<wasm::_Global, wasm::_Module::_GlobalList> wasm::_Module::globals() const {
	return { _Module::_GlobalList{ const_cast<wasm::_Module*>(this) } };
}
wasm::_List<wasm::_Function, wasm::_Module::_FunctionList> wasm::_Module::functions() const {
	return { _Module::_FunctionList{ const_cast<wasm::_Module*>(this) } };
}
