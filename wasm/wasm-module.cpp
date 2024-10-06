#include "wasm-module.h"
#include "wasm-sink.h"

wasm::Module::Module(wasm::ModuleInterface* interface) : pInterface{ interface } {}
wasm::Module::~Module() {
	fClose();
}

wasm::Prototype wasm::Module::fPrototype(std::u8string_view id, std::initializer_list<wasm::Param> params, std::initializer_list<wasm::Type> result) {
	/* validate the id and the parameter */
	std::u8string _id{ id };
	if (!_id.empty() && pPrototype.ids.contains(_id))
		util::fail(u8"Prototype [", _id, u8"] already defined");
	std::unordered_set<std::u8string> names;
	for (const auto& param : params) {
		if (param.id.empty())
			continue;
		if (names.contains(param.id))
			util::fail(u8"Parameter with name [", param.id, u8"] of prototype [", _id, u8"] already defined");
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
wasm::Prototype wasm::Module::fPrototype(std::initializer_list<wasm::Type> params, std::initializer_list<wasm::Type> result) {
	/* check if its the null-type */
	if (params.size() == 0 && result.size() == 0 && pNullPrototype.valid())
		return pNullPrototype;

	/* setup the type-key */
	PrototypeKey key{};
	key.list.insert(key.list.end(), params);
	key.list.insert(key.list.end(), result);
	key.params = params.size();

	/* lookup the type in the map of anonymous-types */
	auto it = pAnonTypes.find(key);
	if (it != pAnonTypes.end())
		return wasm::Prototype{ *this, it->second };

	/* setup the prototype-state */
	detail::PrototypeState state;
	for (wasm::Type param : params)
		state.parameter.emplace_back(param);
	state.result.insert(state.result.end(), result);

	/* allocate the next id and register the next prototype */
	pPrototype.list.push_back(std::move(state));
	wasm::Prototype prototype{ *this, uint32_t(pPrototype.list.size() - 1) };

	/* check if this is the null-type and otherwise insert it into the map */
	if (params.size() == 0 && result.size() == 0)
		pNullPrototype = prototype;
	else {
		std::pair<PrototypeKey, uint32_t> value{ std::move(key), prototype.index() };
		pAnonTypes.insert(std::move(value));
	}

	/* notify the interface about the added prototype */
	pInterface->addPrototype(prototype);
	return prototype;
}
wasm::Function wasm::Module::fFunction(std::u8string_view id, const wasm::Prototype& prototype, const wasm::Exchange& exchange) {
	/* validate the import/export parameter */
	if ((!exchange.importModule.empty() || exchange.exported) && id.empty())
		util::fail(u8"Importing or exporting requires explicit id names");

	/* validate the imports */
	if (exchange.importModule.empty())
		pImportsClosed = true;
	else if (pImportsClosed)
		util::fail(u8"Cannot import function [", id, u8"] after the first non-import object has been added");

	/* validate the id and the prototype */
	std::u8string _id{ id };
	if (!_id.empty() && pFunction.ids.contains(_id))
		util::fail(u8"Function [", _id, u8"] already defined");
	if (!prototype.valid())
		util::fail(u8"Prototype for function [", _id, u8"] must be constructed");
	if (&prototype.module() != this)
		util::fail(u8"Prototype for function [", _id, u8"] must originate from this module");

	/* setup the function */
	detail::FunctionState state = { std::u8string{ exchange.importModule }, {}, prototype, 0, exchange.exported, false };

	/* allocate the next id and register the next function */
	if (!_id.empty())
		state.id = *pFunction.ids.insert(_id).first;
	pFunction.list.push_back(std::move(state));
	wasm::Function function{ *this, uint32_t(pFunction.list.size() - 1) };

	/* notify the interface about the added function */
	pInterface->addFunction(function);
	return function;
}
void wasm::Module::fCheckClosed() const {
	if (pClosed)
		util::fail(u8"Cannot change the closed module");
}
void wasm::Module::fClose() {
	if (pClosed)
		return;
	pClosed = true;

	/* check that all globals have been assigned */
	for (size_t i = 0; i < pGlobal.list.size(); ++i) {
		if (!pGlobal.list[i].importModule.empty())
			continue;
		if (!pGlobal.list[i].assigned)
			util::fail(u8"Global [", wasm::Global{ *this, uint32_t(i) }.toString(), u8"] requires to either be imported or a value assigned to it");
	}

	/* instantiate all sinks to non-sunken functions and close all other remaining sinks */
	for (size_t i = 0; i < pFunction.list.size(); ++i) {
		if (!pFunction.list[i].bound && pFunction.list[i].importModule.empty())
			wasm::Sink _sink{ wasm::Function(*this, uint32_t(i)) };
		if (pFunction.list[i].sink != 0)
			pFunction.list[i].sink->fClose();
	}

	/* mark the module as closed */
	pInterface->close(*this);
}

wasm::Prototype wasm::Module::prototype(std::initializer_list<wasm::Type> params, std::initializer_list<wasm::Type> result) {
	fCheckClosed();
	return fPrototype(params, result);
}
wasm::Prototype wasm::Module::prototype(std::u8string_view id, std::initializer_list<wasm::Param> params, std::initializer_list<wasm::Type> result) {
	fCheckClosed();
	return fPrototype(id, params, result);
}
wasm::Memory wasm::Module::memory(std::u8string_view id, const wasm::Limit& limit, const wasm::Exchange& exchange) {
	fCheckClosed();

	/* validate the import/export parameter */
	if ((!exchange.importModule.empty() || exchange.exported) && id.empty())
		util::fail(u8"Importing or exporting requires explicit id names");

	/* validate the imports */
	if (exchange.importModule.empty())
		pImportsClosed = true;
	else if (pImportsClosed)
		util::fail(u8"Cannot import memory [", id, u8"] after the first non-import object has been added");

	/* validate the id */
	std::u8string _id{ id };
	if (!_id.empty() && pMemory.ids.contains(_id))
		util::fail(u8"Memory [", _id, u8"] already defined");

	/* setup the memory-state */
	detail::MemoryState state = { std::u8string{ exchange.importModule }, limit, {}, exchange.exported };

	/* allocate the next id and register the next memory */
	if (!_id.empty())
		state.id = *pMemory.ids.insert(_id).first;
	pMemory.list.push_back(std::move(state));
	wasm::Memory memory{ *this, uint32_t(pMemory.list.size() - 1) };

	/* notify the interface about the added memory */
	pInterface->addMemory(memory);
	return memory;
}
wasm::Table wasm::Module::table(std::u8string_view id, bool functions, const wasm::Limit& limit, const wasm::Exchange& exchange) {
	fCheckClosed();

	/* validate the import/export parameter */
	if ((!exchange.importModule.empty() || exchange.exported) && id.empty())
		util::fail(u8"Importing or exporting requires explicit id names");

	/* validate the imports */
	if (exchange.importModule.empty())
		pImportsClosed = true;
	else if (pImportsClosed)
		util::fail(u8"Cannot import table [", id, u8"] after the first non-import object has been added");

	/* validate the id */
	std::u8string _id{ id };
	if (!_id.empty() && pTable.ids.contains(_id))
		util::fail(u8"Table [", _id, u8"] already defined");

	/* setup the table-state */
	detail::TableState state = { std::u8string{ exchange.importModule }, limit, {}, exchange.exported, functions };

	/* allocate the next id and register the next table */
	if (!_id.empty())
		state.id = *pTable.ids.insert(_id).first;
	pTable.list.push_back(std::move(state));
	wasm::Table table{ *this, uint32_t(pTable.list.size() - 1) };

	/* notify the interface about the added table */
	pInterface->addTable(table);
	return table;
}
wasm::Global wasm::Module::global(std::u8string_view id, wasm::Type type, bool mutating, const wasm::Exchange& exchange) {
	fCheckClosed();

	/* validate the import/export parameter */
	if ((!exchange.importModule.empty() || exchange.exported) && id.empty())
		util::fail(u8"Importing or exporting requires explicit id names");

	/* validate the imports */
	if (exchange.importModule.empty())
		pImportsClosed = true;
	else if (pImportsClosed)
		util::fail(u8"Cannot import global [", id, u8"] after the first non-import object has been added");

	/* validate the id */
	std::u8string _id{ id };
	if (!_id.empty() && pGlobal.ids.contains(_id))
		util::fail(u8"Global [", _id, u8"] already defined");

	/* setup the global */
	detail::GlobalState state = { std::u8string{ exchange.importModule }, {}, type, exchange.exported, mutating, false };

	/* allocate the next id and register the next global */
	if (!_id.empty())
		state.id = *pGlobal.ids.insert(_id).first;
	pGlobal.list.push_back(std::move(state));
	wasm::Global global{ *this, uint32_t(pGlobal.list.size() - 1) };

	/* notify the interface about the added global */
	pInterface->addGlobal(global);
	return global;
}
wasm::Function wasm::Module::function(std::u8string_view id, const wasm::Prototype& prototype, const wasm::Exchange& exchange) {
	fCheckClosed();
	return fFunction(id, prototype, exchange);
}
wasm::Function wasm::Module::function(std::u8string_view id, std::initializer_list<wasm::Type> params, std::initializer_list<wasm::Type> result, const wasm::Exchange& exchange) {
	fCheckClosed();
	return fFunction(id, fPrototype(params, result), exchange);
}
void wasm::Module::value(const wasm::Global& global, const wasm::Value& value) {
	/* validate the global */
	if (!global.valid())
		util::fail(u8"Global is required to be constructed to set its value");
	if (&global.module() != this)
		util::fail(u8"Global [", global.toString(), u8"] must originate from this module");
	if (global.imported())
		util::fail(u8"Global [", global.toString(), u8"] cannot be set a value as it is being imported");

	/* validate the value */
	wasm::Type _type{};
	switch (value.type()) {
	case wasm::ValType::i32:
		_type = wasm::Type::i32;
		break;
	case wasm::ValType::i64:
		_type = wasm::Type::i64;
		break;
	case wasm::ValType::f32:
		_type = wasm::Type::f32;
		break;
	case wasm::ValType::f64:
		_type = wasm::Type::f64;
		break;
	case wasm::ValType::refExtern:
		_type = wasm::Type::refExtern;
		break;
	case wasm::ValType::refFunction:
		_type = wasm::Type::refFunction;
		if (value.function().valid() && &value.function().module() != this)
			util::fail(u8"Function value for global [", global.toString(), u8"] must originate from this module");
		break;
	case wasm::ValType::global:
		if (!value.global().valid())
			util::fail(u8"Imported value for global [", global.toString(), u8"] must be constructed");
		if (&value.global().module() != this)
			util::fail(u8"Imported value for global [", global.toString(), u8"] must originate from this module");
		if (!value.global().imported() || value.global().mutating())
			util::fail(u8"Imported value for global [", global.toString(), u8"] must be imported and immutable");
		_type = value.global().type();
		break;
	case wasm::ValType::invalid:
		util::fail(u8"Value for global [", global.toString(), u8"] is required to be constructed");
		break;
	}
	if (_type != global.type())
		util::fail(u8"Value for global [", global.toString(), u8"] must match its type");

	/* check if a value has already been assigned to the global */
	if (pGlobal.list[global.index()].assigned)
		util::fail(u8"Value for global [", global.toString(), u8"] can only be assigned once");

	/* mark the value as written and notify the interface */
	pGlobal.list[global.index()].assigned = true;
	pInterface->setValue(global, value);
}
void wasm::Module::data(const wasm::Memory& memory, const wasm::Value& offset, const std::vector<uint8_t>& data) {
	/* validate the memory */
	if (!memory.valid())
		util::fail(u8"Memory is required to be constructed to write data to it");
	if (&memory.module() != this)
		util::fail(u8"Memory [", memory.toString(), u8"] must originate from this module");

	/* validate the offset */
	if (!offset.valid())
		util::fail(u8"Offset to write to memory [", memory.toString(), u8"] is required to be constructed");
	if (offset.type() != wasm::ValType::i32 && offset.type() != wasm::ValType::global)
		util::fail(u8"Offset to write to memory [", memory.toString(), u8"] must be of type i32 or a global-import");
	if (offset.type() == wasm::ValType::global) {
		if (!offset.global().valid())
			util::fail(u8"Imported offset to write to memory [", memory.toString(), u8"] must be constructed");
		if (&offset.global().module() != this)
			util::fail(u8"Imported offset to write to memory [", memory.toString(), u8"] must originate from this module");
		if (offset.global().type() != wasm::Type::i32 || !offset.global().imported() || offset.global().mutating())
			util::fail(u8"Imported offset to write to memory [", memory.toString(), u8"] must be an immutable imported i32");
	}

	/* pass the validated data to the interface */
	pInterface->writeData(memory, offset, data);
}
void wasm::Module::elements(const wasm::Table& table, const wasm::Value& offset, const std::vector<wasm::Value>& values) {
	/* validate the memory */
	if (!table.valid())
		util::fail(u8"Table is required to be constructed to write elements to it");
	if (&table.module() != this)
		util::fail(u8"Table [", table.toString(), u8"] must originate from this module");

	/* validate the offset */
	if (!offset.valid())
		util::fail(u8"Offset to write to table [", table.toString(), u8"] is required to be constructed");
	if (offset.type() != wasm::ValType::i32 && offset.type() != wasm::ValType::global)
		util::fail(u8"Offset to write to table [", table.toString(), u8"] must be of type i32 or a global-import");
	if (offset.type() == wasm::ValType::global) {
		if (!offset.global().valid())
			util::fail(u8"Imported offset to write to table [", table.toString(), u8"] must be constructed");
		if (&offset.global().module() != this)
			util::fail(u8"Imported offset to write to table [", table.toString(), u8"] must originate from this module");
		if (offset.global().type() != wasm::Type::i32 || !offset.global().imported() || offset.global().mutating())
			util::fail(u8"Imported offset to write to table [", table.toString(), u8"] must be an immutable imported i32");
	}

	/* validate the values */
	wasm::Type _type{};
	for (const wasm::Value& value : values) {
		switch (value.type()) {
		case wasm::ValType::i32:
			_type = wasm::Type::i32;
			break;
		case wasm::ValType::i64:
			_type = wasm::Type::i64;
			break;
		case wasm::ValType::f32:
			_type = wasm::Type::f32;
			break;
		case wasm::ValType::f64:
			_type = wasm::Type::f64;
			break;
		case wasm::ValType::refExtern:
			_type = wasm::Type::refExtern;
			break;
		case wasm::ValType::refFunction:
			_type = wasm::Type::refFunction;
			if (value.function().valid() && &value.function().module() != this)
				util::fail(u8"Function value for table [", table.toString(), u8"] must originate from this module");
			break;
		case wasm::ValType::global:
			if (!value.global().valid())
				util::fail(u8"Imported value for table [", table.toString(), u8"] must be constructed");
			if (&value.global().module() != this)
				util::fail(u8"Imported value for table [", table.toString(), u8"] must originate from this module");
			if (!value.global().imported() || value.global().mutating())
				util::fail(u8"Imported value for table [", table.toString(), u8"] must be imported and immutable");
			_type = value.global().type();
			break;
		case wasm::ValType::invalid:
			util::fail(u8"Value for table [", table.toString(), u8"] is required to be constructed");
			break;
		}
		if (_type != (table.functions() ? wasm::Type::refFunction : wasm::Type::refExtern))
			util::fail(u8"Value for table [", table.toString(), u8"] must match its type");
	}

	/* pass the validated data to the interface */
	pInterface->writeElements(table, offset, values);
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
