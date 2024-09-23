#include "text-module.h"

writer::text::Module::Module(const writer::TextWriter& state) {}
writer::text::Prototype writer::text::Module::addPrototype(const std::u8string_view& id, const wasm::Param* params, size_t paramCount, const wasm::Type* results, size_t resultCount) {
	/* validate the id */
	std::u8string _id{ id };
	if (!_id.empty() && pTypes.ids.count(_id) != 0)
		util::fail(u8"Prototype with id [", _id, u8"] already defined");

	/* allocate the next id-object */
	if (!_id.empty()) {
		pTypes.ids.insert(_id);
		_id = str::Build<std::u8string>(u8"$", _id);
	}
	text::IdObject obj{ _id, pTypes.next++ };

	/* validate the uniqueness of all of the names */
	std::unordered_set<std::u8string> names;
	for (size_t i = 0; i < paramCount; ++i) {
		if (params[i].id.empty())
			continue;
		if (names.count(params[i].id) != 0)
			util::fail(u8"Parameter with id [", params[i].id, u8"] already defined");
		names.insert(params[i].id);
	}

	/* write the actual type to the output */
	str::BuildTo(pBody, u8"\n  (type", text::MakeId(id), u8" (func");
	for (size_t i = 0; i < paramCount; ++i)
		str::BuildTo(pBody, u8" (param", text::MakeId(params[i].id), text::MakeType(params[i].type), u8')');
	if (resultCount > 0) {
		pBody.append(u8" (result");
		for (size_t i = 0; i < resultCount; ++i)
			str::BuildTo(pBody, text::MakeType(results[i]));
		pBody.push_back(u8')');
	}
	pBody.append(u8"))");

	/* register the parameters to the parameter-list */
	pParameter.emplace_back(params, params + paramCount);
	return obj;
}
writer::text::Memory writer::text::Module::addMemory(const std::u8string_view& id, const wasm::Limit& limit, const wasm::Exchange& exchange) {
	/* validate the id */
	std::u8string _id{ id };
	if (!_id.empty() && pMemory.ids.count(_id) != 0)
		util::fail(u8"Memory with id [", _id, "] already defined");

	/* allocate the next id-object */
	if (!_id.empty()) {
		pMemory.ids.insert(_id);
		_id = str::Build<std::u8string>(u8"$", _id);
	}
	text::IdObject obj{ _id, pMemory.next++ };

	/* write the actual memory to the output */
	str::BuildTo((exchange.impName.empty() ? pBody : pImports), u8"\n  (memory", text::MakeId(id), text::MakeExchange(exchange), text::MakeLimit(limit), u8')');

	return obj;
}
writer::text::Table writer::text::Module::addTable(const std::u8string_view& id, bool functions, const wasm::Limit& limit, const wasm::Exchange& exchange) {
	/* validate the id */
	std::u8string _id{ id };
	if (!_id.empty() && pTables.ids.count(_id) != 0)
		util::fail(u8"Table with id [", _id, "] already defined");

	/* allocate the next id-object */
	if (!_id.empty()) {
		pTables.ids.insert(_id);
		_id = str::Build<std::u8string>(u8"$", _id);
	}
	text::IdObject obj{ _id, pTables.next++ };

	/* write the actual table to the output */
	str::BuildTo((exchange.impName.empty() ? pBody : pImports), u8"\n  (table", text::MakeId(id), text::MakeExchange(exchange), text::MakeLimit(limit), functions ? u8" funcref)" : u8" externref)");

	return obj;
}
writer::text::Global writer::text::Module::addGlobal(const std::u8string_view& id, wasm::Type type, bool mut, const wasm::Exchange& exchange) {
	/* validate the id */
	std::u8string _id{ id };
	if (!_id.empty() && pGlobals.ids.count(_id) != 0)
		util::fail(u8"Globals with id [", _id, "] already defined");

	/* allocate the next id-object */
	if (!_id.empty()) {
		pGlobals.ids.insert(_id);
		_id = str::Build<std::u8string>(u8"$", _id);
	}
	text::IdObject obj{ _id, pTables.next++ };

	/* write the actual global to the output */
	std::u8string typeString = (mut ? str::Build<std::u8string>(u8" (mut", text::MakeType(type), u8')') : std::u8string(text::MakeType(type)));
	str::BuildTo((exchange.impName.empty() ? pBody : pImports), u8"\n  (global", text::MakeId(id), text::MakeExchange(exchange), typeString, u8')');

	return obj;
}
writer::text::Function writer::text::Module::addFunction(const std::u8string_view& id, const text::Prototype* prototype, const wasm::Exchange& exchange) {
	/* validate the id */
	std::u8string _id{ id };
	if (!_id.empty() && pFunctions.ids.count(_id) != 0)
		util::fail(u8"Function with id [", _id, "] already defined");

	/* allocate the next id-object */
	if (!_id.empty()) {
		pFunctions.ids.insert(_id);
		_id = str::Build<std::u8string>(u8"$", _id);
	}
	text::IdObject obj{ _id, pFunctions.next++ };

	/* construct the initial function description */
	std::u8string funcText;
	str::BuildTo(funcText, u8"\n  (func", text::MakeId(id), text::MakeExchange(exchange));
	if (prototype != 0)
		str::BuildTo(funcText, u8" (type ", prototype->toString(), u8')');

	/* check if this is not an import, in which case a proper sink needs to be set-up, otherwise it can be produced immediately */
	if (exchange.impName.empty())
		pSinks.emplace_back(std::move(funcText), (prototype == 0 ? uint32_t(-1) : prototype->index));
	else {
		pSinks.emplace_back();
		pImports.append(funcText);
	}

	return obj;
}
writer::text::SinkWrapper writer::text::Module::bindSink(const text::Function& fn) {
	/* check if the sink is still available */
	if (!pSinks[fn.index].open)
		util::fail(u8"Cannot construct a sink to function [", fn.toString(), "] as it is marked as an import");
	std::u8string content;
	std::swap(content, pSinks[fn.index].header);
	pSinks[fn.index].open = false;

	/* setup the next instance of the sink */
	std::vector<wasm::Param> parameter;
	if (pSinks[fn.index].parameter != uint32_t(-1))
		parameter = pParameter[pSinks[fn.index].parameter];
	return text::SinkWrapper{ &pSink, pSink.nextInstance(std::move(content), std::move(parameter), pBody) };
}
std::u8string writer::text::Module::toString() {
	/* flush all incomplete functions out (no need to copy the parameter as no instructions will be added) */
	for (size_t i = 0; i < pSinks.size(); ++i) {
		if (pSinks[i].open)
			pSink.nextInstance(pSinks[i].header, {}, pBody);
		pSinks[i].open = false;
	}

	/* close the current sink (by simply allocating an empty next instance) */
	pSink.nextInstance(u8"", {}, pBody);

	/* construct the total output */
	if (pImports.empty() && pBody.empty())
		return u8"(module)";
	return str::Build<std::u8string>(u8"(module", pImports, pBody, u8"\n)");
}
