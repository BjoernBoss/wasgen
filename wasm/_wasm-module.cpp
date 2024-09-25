#include "_wasm-module.h"
#include "_wasm-objects.h"

#include "../util/logging.h"

wasm::_Prototype wasm::_Module::prototype(std::u8string_view id, std::initializer_list<wasm::Param> params, std::initializer_list<wasm::Type> result) {
	std::u8string _id{ id };

	/* validate the id */
	if (!_id.empty() && pPrototype.ids.contains(_id))
		util::fail(u8"Prototype with id [", _id, u8"] already defined");

	/* setup and validate the prototype */
	detail::PrototypeState state;
	std::unordered_set<std::u8string> names;
	for (const auto& param : params) {
		if (param.id.empty())
			continue;
		if (names.contains(param.id))
			util::fail(u8"Parameter with name [", param.id, u8"] of prototype with id [", _id, u8"] already defined");
		names.insert(param.id);
	}
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

	/* setup the memory */
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

	/* setup the table */
	detail::TableState state = { imported, exported, limit, {}, functions };

	/* allocate the next id and register the next table */
	if (!_id.empty())
		state.id = *pTable.ids.insert(_id).first;
	pTable.list.push_back(std::move(state));
	return wasm::_Table{ *this, uint32_t(pTable.list.size() - 1) };
}
