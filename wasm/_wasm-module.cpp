#include "_wasm-module.h"

#include "../util/logging.h"

wasm::_Prototype wasm::_Module::prototype(std::u8string_view id, std::initializer_list<wasm::_Param> params, std::initializer_list<wasm::_Type> result) {
	std::u8string _id{ id };

	/* validate the id and the parameter */
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
	return wasm::_Prototype{ *this, uint32_t(pPrototype.list.size() - 1) };
}
wasm::_Memory wasm::_Module::memory(std::u8string_view id, const wasm::_Limit& limit, const wasm::_Import& imported, const wasm::_Export& exported) {
	std::u8string _id{ id };

	/* validate the id */
	if (!_id.empty() && pMemory.ids.contains(_id))
		util::fail(u8"Memory with id [", _id, u8"] already defined");

	/* setup the memory-state */
	detail::MemoryState state = { imported, exported, limit, {} };

	/* allocate the next id and register the next memory */
	if (!_id.empty())
		state.id = *pMemory.ids.insert(_id).first;
	pMemory.list.push_back(std::move(state));
	return wasm::_Memory{ *this, uint32_t(pMemory.list.size() - 1) };
}
wasm::_Table wasm::_Module::table(std::u8string_view id, bool functions, const wasm::_Limit& limit, const wasm::_Import& imported, const wasm::_Export& exported) {
	std::u8string _id{ id };

	/* validate the id */
	if (!_id.empty() && pTable.ids.contains(_id))
		util::fail(u8"Table with id [", _id, u8"] already defined");

	/* setup the table-state */
	detail::TableState state = { imported, exported, limit, {}, functions };

	/* allocate the next id and register the next table */
	if (!_id.empty())
		state.id = *pTable.ids.insert(_id).first;
	pTable.list.push_back(std::move(state));
	return wasm::_Table{ *this, uint32_t(pTable.list.size() - 1) };
}
wasm::_Global wasm::_Module::global(wasm::_Type type, bool mutating, std::u8string_view id, const wasm::_Import& imported, const wasm::_Export& exported) {
	std::u8string _id{ id };

	/* validate the id-state */
	if (!_id.empty() && pGlobal.ids.contains(_id))
		util::fail(u8"Global with id [", _id, u8"] already defined");

	/* setup the global */
	detail::GlobalState state = { imported, exported, {}, type, mutating };

	/* allocate the next id and register the next global */
	if (!_id.empty())
		state.id = *pGlobal.ids.insert(_id).first;
	pGlobal.list.push_back(std::move(state));
	return wasm::_Global{ *this, uint32_t(pGlobal.list.size() - 1) };
}
wasm::_Function wasm::_Module::function(std::u8string_view id, const wasm::_Prototype& prototype, const wasm::_Import& imported, const wasm::_Export& exported) {
	std::u8string _id{ id };

	/* validate the id and the prototype */
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
	return wasm::_Function{ *this, uint32_t(pFunction.list.size() - 1) };
}
