/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2024 Bjoern Boss Henrichsen */
#include "wasm-module.h"
#include "../sink/wasm-sink.h"

wasm::Module::Module(wasm::ModuleInterface* interface) : pInterface{ interface } {}
wasm::Module::~Module() noexcept(false) {
	if (std::uncaught_exceptions() == 0)
		fClose();
}

wasm::Prototype wasm::Module::fPrototype(std::u8string_view id, std::initializer_list<wasm::Param> params, std::initializer_list<wasm::Type> result) {
	/* validate the id and the parameter */
	std::u8string _id{ id };
	if (!_id.empty() && pPrototype.ids.contains(_id))
		throw wasm::Exception{ L"Prototype [", _id, L"] already defined" };
	std::unordered_set<std::u8string> names;
	for (const auto& param : params) {
		if (param.id.empty())
			continue;
		if (names.contains(param.id))
			throw wasm::Exception{ L"Parameter with name [", param.id, L"] of prototype [", _id, L"] already defined" };
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
		throw wasm::Exception{ L"Importing or exporting requires explicit id names" };

	/* validate the imports */
	if (exchange.importModule.empty())
		pImportsClosed = true;
	else if (pImportsClosed)
		throw wasm::Exception{ L"Cannot import function [", id, L"] after the first non-import object has been added" };

	/* validate the id and the prototype */
	std::u8string _id{ id };
	if (!_id.empty() && pFunction.ids.contains(_id))
		throw wasm::Exception{ L"Function [", _id, L"] already defined" };
	if (!prototype.valid())
		throw wasm::Exception{ L"Prototype for function [", _id, L"] must be constructed" };
	if (&prototype.module() != this)
		throw wasm::Exception{ L"Prototype for function [", _id, L"] must originate from this module" };

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
void wasm::Module::fData(const wasm::Memory& memory, const wasm::Value& offset, const uint8_t* data, uint32_t count) {
	/* validate the memory */
	if (!memory.valid())
		throw wasm::Exception{ L"Memory is required to be constructed to write data to it" };
	if (&memory.module() != this)
		throw wasm::Exception{ L"Memory [", memory.toString(), L"] must originate from this module" };

	/* validate the offset */
	if (!offset.valid())
		throw wasm::Exception{ L"Offset to write to memory [", memory.toString(), L"] is required to be constructed" };
	if (offset.type() != wasm::ValType::i32 && offset.type() != wasm::ValType::global)
		throw wasm::Exception{ L"Offset to write to memory [", memory.toString(), L"] must be of type i32 or a global-import" };
	if (offset.type() == wasm::ValType::global) {
		if (!offset.global().valid())
			throw wasm::Exception{ L"Imported offset to write to memory [", memory.toString(), L"] must be constructed" };
		if (&offset.global().module() != this)
			throw wasm::Exception{ L"Imported offset to write to memory [", memory.toString(), L"] must originate from this module" };
		if (offset.global().type() != wasm::Type::i32 || !offset.global().imported() || offset.global().mutating())
			throw wasm::Exception{ L"Imported offset to write to memory [", memory.toString(), L"] must be an immutable imported i32" };
	}

	/* pass the validated data to the interface */
	pInterface->writeData(memory, offset, data, count);
}
void wasm::Module::fElements(const wasm::Table& table, const wasm::Value& offset, const wasm::Value* values, uint32_t count) {
	/* validate the memory */
	if (!table.valid())
		throw wasm::Exception{ L"Table is required to be constructed to write elements to it" };
	if (&table.module() != this)
		throw wasm::Exception{ L"Table [", table.toString(), L"] must originate from this module" };

	/* validate the offset */
	if (!offset.valid())
		throw wasm::Exception{ L"Offset to write to table [", table.toString(), L"] is required to be constructed" };
	if (offset.type() != wasm::ValType::i32 && offset.type() != wasm::ValType::global)
		throw wasm::Exception{ L"Offset to write to table [", table.toString(), L"] must be of type i32 or a global-import" };
	if (offset.type() == wasm::ValType::global) {
		if (!offset.global().valid())
			throw wasm::Exception{ L"Imported offset to write to table [", table.toString(), L"] must be constructed" };
		if (&offset.global().module() != this)
			throw wasm::Exception{ L"Imported offset to write to table [", table.toString(), L"] must originate from this module" };
		if (offset.global().type() != wasm::Type::i32 || !offset.global().imported() || offset.global().mutating())
			throw wasm::Exception{ L"Imported offset to write to table [", table.toString(), L"] must be an immutable imported i32" };
	}

	/* validate the values */
	wasm::Type _type{};
	for (uint32_t i = 0; i < count; ++i) {
		const wasm::Value& value = values[i];

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
				throw wasm::Exception{ L"Function value for table [", table.toString(), L"] must originate from this module" };
			break;
		case wasm::ValType::global:
			if (!value.global().valid())
				throw wasm::Exception{ L"Imported value for table [", table.toString(), L"] must be constructed" };
			if (&value.global().module() != this)
				throw wasm::Exception{ L"Imported value for table [", table.toString(), L"] must originate from this module" };
			if (!value.global().imported() || value.global().mutating())
				throw wasm::Exception{ L"Imported value for table [", table.toString(), L"] must be imported and immutable" };
			_type = value.global().type();
			break;
		case wasm::ValType::invalid:
			throw wasm::Exception{ L"Value for table [", table.toString(), L"] is required to be constructed" };
		}
		if (_type != (table.functions() ? wasm::Type::refFunction : wasm::Type::refExtern))
			throw wasm::Exception{ L"Value for table [", table.toString(), L"] must match its type" };
	}

	/* pass the validated data to the interface */
	pInterface->writeElements(table, offset, values, count);
}
void wasm::Module::fCheck() const {
	/* check if any queued exceptions need to be thrown */
	if (!pException.empty()) {
		std::wstring err;
		std::swap(err, pException);
		throw wasm::Exception{ err };
	}

	/* check if the sink has already been closed */
	if (pClosed)
		throw wasm::Exception{ L"Cannot change the closed module" };
}
void wasm::Module::fClose() {
	if (pClosed)
		return;

	/* process any queued exceptions and afterwards mark the module
	*	as closed (otherwise checking will throw an exception) */
	fCheck();
	pClosed = true;

	/* check that all memory-limits have been set and otherwise default them */
	for (size_t i = 0; i < pMemory.list.size(); ++i) {
		if (!pMemory.list[i].importModule.empty())
			continue;
		if (!pMemory.list[i].limit.valid())
			throw wasm::Exception{ L"Memory [", wasm::Memory{ *this, uint32_t(i) }.toString(), L"] requires a limit to be set" };
	}

	/* check that all table-limits have been set */
	for (size_t i = 0; i < pTable.list.size(); ++i) {
		if (!pTable.list[i].importModule.empty())
			continue;
		if (!pTable.list[i].limit.valid())
			throw wasm::Exception{ L"Table [", wasm::Table{ *this, uint32_t(i) }.toString(), L"] requires a limit to be set" };
	}

	/* check that all globals have been assigned */
	for (size_t i = 0; i < pGlobal.list.size(); ++i) {
		if (!pGlobal.list[i].importModule.empty())
			continue;
		if (!pGlobal.list[i].assigned)
			throw wasm::Exception{ L"Global [", wasm::Global{ *this, uint32_t(i) }.toString(), L"] requires to either be imported or a value assigned to it" };
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
void wasm::Module::fDeferredException(const wasm::Exception& error) {
	if (pException.empty())
		pException = error.what();
}

wasm::Prototype wasm::Module::prototype(std::initializer_list<wasm::Type> params, std::initializer_list<wasm::Type> result) {
	fCheck();
	return fPrototype(params, result);
}
wasm::Prototype wasm::Module::prototype(std::u8string_view id, std::initializer_list<wasm::Param> params, std::initializer_list<wasm::Type> result) {
	fCheck();
	return fPrototype(id, params, result);
}
wasm::Memory wasm::Module::memory(std::u8string_view id, const wasm::Limit& limit, const wasm::Exchange& exchange) {
	fCheck();

	/* validate the import/export parameter */
	if ((!exchange.importModule.empty() || exchange.exported) && id.empty())
		throw wasm::Exception{ L"Importing or exporting requires explicit id names" };

	/* validate the imports */
	if (exchange.importModule.empty())
		pImportsClosed = true;
	else if (pImportsClosed)
		throw wasm::Exception{ L"Cannot import memory [", id, L"] after the first non-import object has been added" };

	/* validate the limit */
	if (!exchange.importModule.empty() && !limit.valid())
		throw wasm::Exception{ L"Imported memory [", id, L"] immediately requires a valid limit" };

	/* validate the id */
	std::u8string _id{ id };
	if (!_id.empty() && pMemory.ids.contains(_id))
		throw wasm::Exception{ L"Memory [", _id, L"] already defined" };

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
	fCheck();

	/* validate the import/export parameter */
	if ((!exchange.importModule.empty() || exchange.exported) && id.empty())
		throw wasm::Exception{ L"Importing or exporting requires explicit id names" };

	/* validate the imports */
	if (exchange.importModule.empty())
		pImportsClosed = true;
	else if (pImportsClosed)
		throw wasm::Exception{ L"Cannot import table [", id, L"] after the first non-import object has been added" };

	/* validate the limit */
	if (!exchange.importModule.empty() && !limit.valid())
		throw wasm::Exception{ L"Imported table [", id, L"] immediately requires a valid limit" };

	/* validate the id */
	std::u8string _id{ id };
	if (!_id.empty() && pTable.ids.contains(_id))
		throw wasm::Exception{ L"Table [", _id, L"] already defined" };

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
	fCheck();

	/* validate the import/export parameter */
	if ((!exchange.importModule.empty() || exchange.exported) && id.empty())
		throw wasm::Exception{ L"Importing or exporting requires explicit id names" };

	/* validate the imports */
	if (exchange.importModule.empty())
		pImportsClosed = true;
	else if (pImportsClosed)
		throw wasm::Exception{ L"Cannot import global [", id, L"] after the first non-import object has been added" };

	/* validate the id */
	std::u8string _id{ id };
	if (!_id.empty() && pGlobal.ids.contains(_id))
		throw wasm::Exception{ L"Global [", _id, L"] already defined" };

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
	fCheck();
	return fFunction(id, prototype, exchange);
}
wasm::Function wasm::Module::function(std::u8string_view id, std::initializer_list<wasm::Type> params, std::initializer_list<wasm::Type> result, const wasm::Exchange& exchange) {
	fCheck();
	return fFunction(id, fPrototype(params, result), exchange);
}
void wasm::Module::startup(const wasm::Function& function) {
	fCheck();

	/* validate the function */
	if (!function.valid())
		throw wasm::Exception{ L"Function is required to be constructed to use it as startup" };
	if (&function.module() != this)
		throw wasm::Exception{ L"Function [", function.toString(), L"] must originate from this module" };

	/* check if a startup has already been set */
	if (pHasStartup)
		throw wasm::Exception{ L"Function [", function.toString(), L"] cannot be set as startup as only one startup is allowed per module" };

	/* mark the startup as set and notify the interface */
	pHasStartup = true;
	pInterface->setStartup(function);
}
void wasm::Module::limit(const wasm::Memory& memory, const wasm::Limit& limit) {
	fCheck();

	/* validate the memory */
	if (!memory.valid())
		throw wasm::Exception{ L"Memory is required to be constructed to set its limit" };
	if (&memory.module() != this)
		throw wasm::Exception{ L"Memory [", memory.toString(), L"] must originate from this module" };

	/* validate the limit */
	if (!limit.valid())
		throw wasm::Exception{ L"Memory [", memory.toString(), L"] can only be assigned valid limits" };

	/* check if a limit has already been assigned to the memory */
	if (pMemory.list[memory.index()].limit.valid())
		throw wasm::Exception{ L"Limit for memory [", memory.toString(), L"] can only be assigned once" };

	/* mark the limit as written and notify the interface */
	pMemory.list[memory.index()].limit = limit;
	pInterface->setMemoryLimit(memory);
}
void wasm::Module::limit(const wasm::Table& table, const wasm::Limit& limit) {
	fCheck();

	/* validate the table */
	if (!table.valid())
		throw wasm::Exception{ L"Table is required to be constructed to set its limit" };
	if (&table.module() != this)
		throw wasm::Exception{ L"Table [", table.toString(), L"] must originate from this module" };

	/* validate the limit */
	if (!limit.valid())
		throw wasm::Exception{ L"Table [", table.toString(), L"] can only be assigned valid limits" };

	/* check if a limit has already been assigned to the table */
	if (pTable.list[table.index()].limit.valid())
		throw wasm::Exception{ L"Limit for table [", table.toString(), L"] can only be assigned once" };

	/* mark the limit as written and notify the interface */
	pTable.list[table.index()].limit = limit;
	pInterface->setTableLimit(table);
}
void wasm::Module::value(const wasm::Global& global, const wasm::Value& value) {
	fCheck();

	/* validate the global */
	if (!global.valid())
		throw wasm::Exception{ L"Global is required to be constructed to set its value" };
	if (&global.module() != this)
		throw wasm::Exception{ L"Global [", global.toString(), L"] must originate from this module" };
	if (global.imported())
		throw wasm::Exception{ L"Global [", global.toString(), L"] cannot be set a value as it is being imported" };

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
			throw wasm::Exception{ L"Function value for global [", global.toString(), L"] must originate from this module" };
		break;
	case wasm::ValType::global:
		if (!value.global().valid())
			throw wasm::Exception{ L"Imported value for global [", global.toString(), L"] must be constructed" };
		if (&value.global().module() != this)
			throw wasm::Exception{ L"Imported value for global [", global.toString(), L"] must originate from this module" };
		if (!value.global().imported() || value.global().mutating())
			throw wasm::Exception{ L"Imported value for global [", global.toString(), L"] must be imported and immutable" };
		_type = value.global().type();
		break;
	case wasm::ValType::invalid:
		throw wasm::Exception{ L"Value for global [", global.toString(), L"] is required to be constructed" };
	}
	if (_type != global.type())
		throw wasm::Exception{ L"Value for global [", global.toString(), L"] must match its type" };

	/* check if a value has already been assigned to the global */
	if (pGlobal.list[global.index()].assigned)
		throw wasm::Exception{ L"Value for global [", global.toString(), L"] can only be assigned once" };

	/* mark the value as written and notify the interface */
	pGlobal.list[global.index()].assigned = true;
	pInterface->setValue(global, value);
}
void wasm::Module::data(const wasm::Memory& memory, const wasm::Value& offset, const std::vector<uint8_t>& data) {
	fCheck();
	fData(memory, offset, data.data(), uint32_t(data.size()));
}
void wasm::Module::data(const wasm::Memory& memory, const wasm::Value& offset, const uint8_t* data, size_t count) {
	fCheck();
	fData(memory, offset, data, uint32_t(count));
}
void wasm::Module::elements(const wasm::Table& table, const wasm::Value& offset, const std::vector<wasm::Value>& values) {
	fCheck();
	fElements(table, offset, values.data(), uint32_t(values.size()));
}
void wasm::Module::elements(const wasm::Table& table, const wasm::Value& offset, const wasm::Value* values, size_t count) {
	fCheck();
	fElements(table, offset, values, uint32_t(count));
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
