#include "wasm-sink.h"
#include "wasm-module.h"

wasm::Sink::Sink(const wasm::Function& function) {
	/* validate that the function can be used as sink-target */
	if (!function.valid())
		util::fail(u8"Functions must be constructed to create a sink to them");
	if (function.imported().valid())
		util::fail(u8"Sinks cannot be created for imported function [", function.toString(), u8"]");
	wasm::Prototype prototype = function.prototype();

	/* check if the module is closed or the function has already been bound */
	wasm::Module& module = wasm::Function{ function }.module();
	module.fCheckClosed();
	if (module.pFunction.list[function.index()].bound)
		util::fail(u8"Sink cannot be created for function [", function.toString(), u8"] for which a sink has already been created before");
	module.pFunction.list[function.index()].bound = true;

	/* setup the sink-state */
	pFunction = function;
	if (prototype.valid()) {
		const auto& params = prototype.parameter();
		for (size_t i = 0; i < params.size(); ++i) {
			pVariables.list.push_back({ {}, params[i].type });
			if (!params[i].id.empty())
				pVariables.list.back().id = *pVariables.ids.insert(params[i].id).first;
		}
	}
	pParameter = uint32_t(pVariables.list.size());
	module.pFunction.list[function.index()].sink = this;

	/* setup the sink-interface */
	pInterface = module.pInterface->sink(pFunction);
}
wasm::Sink::~Sink() {
	fClose();
}

std::u8string wasm::Sink::fError() const {
	return str::Build<std::u8string>(u8"Error in sink to function [", pFunction.toString(), u8"]: ");
}
void wasm::Sink::fClose() {
	if (pClosed)
		return;
	pClosed = true;

	/* close all remaining scopes and unregister the sink from the function */
	fPopUntil(0);
	pFunction.module().pFunction.list[pFunction.index()].sink = 0;

	/* mark the sink as closed */
	pInterface->close(*this);
}
void wasm::Sink::fCheckClosed() const {
	if (pClosed)
		util::fail(fError(), u8"Cannot change the closed");
}
void wasm::Sink::fPopUntil(uint32_t size) {
	while (pTargets.size() > size) {
		pInterface->popScope(pTargets.back().type);
		pTargets.pop_back();
	}
}
bool wasm::Sink::fCheckTarget(uint32_t index, size_t stamp, bool soft) const {
	if (index < pTargets.size() && pTargets[index].stamp == stamp)
		return true;
	if (!soft)
		util::fail(fError(), u8"Target [", index, u8"] is out of scope");
	return false;
}
void wasm::Sink::fSetupTarget(const wasm::Prototype& prototype, std::u8string_view id, wasm::ScopeType type, wasm::Target& target) {
	fCheckClosed();

	/* validate the prototype */
	if (prototype.valid() && &prototype.module() != &pFunction.module())
		util::fail(fError(), u8"Prototype [", prototype.toString(), u8"] must originate from same module as function");

	/* no need to validate the uniqueness of the id, as the name can be duplicated */
	detail::TargetState state = { prototype, std::u8string{ id }, ++pNextStamp, type, false };
	pTargets.push_back(std::move(state));
	uint32_t index = uint32_t(pTargets.size() - 1);

	/* configure the target */
	target.pIndex = index;
	target.pStamp = pNextStamp;

	/* notify the interface about the added scope */
	pInterface->pushScope(target);
}
void wasm::Sink::fToggleTarget(uint32_t index, size_t stamp) {
	/* ignore the target if its already out of scope or already toggled */
	if (index >= pTargets.size() || pTargets[index].stamp != stamp)
		return;
	if (pTargets[index].type != wasm::ScopeType::conditional || pTargets[index].otherwise)
		return;

	/* pop all intermediate objects and toggle the target */
	fPopUntil(index + 1);
	pTargets[index].otherwise = true;

	/* notify the interface about the changed scope */
	pInterface->toggleConditional();
}
void wasm::Sink::fCloseTarget(uint32_t index, size_t stamp) {
	/* ignore the target if its already out of scope */
	if (index < pTargets.size() && pTargets[index].stamp == stamp)
		fPopUntil(index);
}

wasm::Variable wasm::Sink::parameter(uint32_t index) {
	if (index >= pParameter)
		util::fail(fError(), u8"Parameter index [", index, u8"] out of bounds");
	return wasm::Variable{ *this, index };
}
wasm::Variable wasm::Sink::local(wasm::Type type, std::u8string_view id) {
	fCheckClosed();

	/* validate the id */
	std::u8string _id{ id };
	if (!_id.empty() && pVariables.ids.contains(_id))
		util::fail(fError(), u8"Variable with id [", _id, u8"] already defined in sink");

	/* setup the variable-state */
	detail::VariableState state = { {}, type };

	/* allocate the next id and register the next variable */
	if (!_id.empty())
		state.id = *pVariables.ids.insert(_id).first;
	pVariables.list.push_back(std::move(state));
	wasm::Variable variable{ *this, uint32_t(pVariables.list.size() - 1) };

	/* notify the interface about the added variable */
	pInterface->addLocal(variable);
	return variable;
}
wasm::Function wasm::Sink::function() const {
	return pFunction;
}
void wasm::Sink::close() {
	fClose();
}

wasm::List<wasm::Variable, wasm::Sink::LocalList> wasm::Sink::locals() const {
	return { Sink::LocalList{ const_cast<wasm::Sink*>(this) } };
}

void wasm::Sink::operator[](const wasm::InstConst& inst) {
	fCheckClosed();
	pInterface->addInst(inst);
}
void wasm::Sink::operator[](const wasm::InstSimple& inst) {
	fCheckClosed();
	pInterface->addInst(inst);
}
void wasm::Sink::operator[](const wasm::InstMemory& inst) {
	fCheckClosed();

	/* validate the instruction-operands */
	if (inst.memory.valid() && &inst.memory.module() != &pFunction.module())
		util::fail(fError(), u8"Memory [", inst.memory.toString(), u8"] must originate from same module as function");
	if (inst.type == wasm::InstMemory::Type::copy && inst.destination.valid()) {
		if (&inst.destination.module() != &pFunction.module())
			util::fail(fError(), u8"Memory [", inst.destination.toString(), u8"] must originate from same module as function");
		if (!inst.memory.valid())
			util::fail(fError(), u8"Memory copy operation using two separate memories must name both explicitly to prevent ambiguities");
	}

	/* add the instruction to the interface */
	pInterface->addInst(inst);
}
void wasm::Sink::operator[](const wasm::InstTable& inst) {
	fCheckClosed();

	/* validate the instruction-operands */
	if (inst.table.valid() && &inst.table.module() != &pFunction.module())
		util::fail(fError(), u8"Table [", inst.table.toString(), u8"] must originate from same module as function");
	if (inst.type == wasm::InstTable::Type::copy && inst.destination.valid()) {
		if (&inst.destination.module() != &pFunction.module())
			util::fail(fError(), u8"Table [", inst.destination.toString(), u8"] must originate from same module as function");
		if (!inst.table.valid())
			util::fail(fError(), u8"Table copy operation using two separate tables must name both explicitly to prevent ambiguities");
	}

	/* add the instruction to the interface */
	pInterface->addInst(inst);
}
void wasm::Sink::operator[](const wasm::InstLocal& inst) {
	fCheckClosed();

	/* validate the instruction-operands */
	if (!inst.variable.valid())
		util::fail(fError(), u8"Locals must be constructed");
	if (&inst.variable.sink() != this)
		util::fail(fError(), u8"Local [", inst.variable.toString(), u8"] must originate from sink");

	/* add the instruction to the interface */
	pInterface->addInst(inst);
}
void wasm::Sink::operator[](const wasm::InstGlobal& inst) {
	fCheckClosed();

	/* validate the instruction-operands */
	if (!inst.global.valid())
		util::fail(fError(), u8"Globals must be constructed");
	if (&inst.global.module() != &pFunction.module())
		util::fail(fError(), u8"Global [", inst.global.toString(), u8"] must originate from same module as function");

	/* add the instruction to the interface */
	pInterface->addInst(inst);
}
void wasm::Sink::operator[](const wasm::InstFunction& inst) {
	fCheckClosed();

	/* validate the instruction-operands */
	if (!inst.function.valid())
		util::fail(fError(), u8"Functions must be constructed");
	if (&inst.function.module() != &pFunction.module())
		util::fail(fError(), u8"Function [", inst.function.toString(), u8"] must originate from same module as function");

	/* add the instruction to the interface */
	pInterface->addInst(inst);
}
void wasm::Sink::operator[](const wasm::InstIndirect& inst) {
	fCheckClosed();

	/* validate the instruction-operands */
	if (inst.table.valid() && &inst.table.module() != &pFunction.module())
		util::fail(fError(), u8"Table [", inst.table.toString(), u8"] must originate from same module as function");
	if (inst.prototype.valid() && &inst.prototype.module() != &pFunction.module())
		util::fail(fError(), u8"Prototype [", inst.prototype.toString(), u8"] must originate from same module as function");

	/* add the instruction to the interface */
	pInterface->addInst(inst);
}
void wasm::Sink::operator[](const wasm::InstBranch& inst) {
	fCheckClosed();

	/* validate the instruction-operands */
	if (!inst.target.valid())
		util::fail(fError(), u8"Targets must be constructed and not out of scope");
	if (&inst.target.sink() != this)
		util::fail(fError(), u8"Target [", inst.target.toString(), u8"] must originate from sink");
	if (inst.type == wasm::InstBranch::Type::table) {
		for (size_t i = 0; i < inst.list.size(); ++i) {
			const wasm::Target& target = inst.list.begin()[i];

			if (!target.valid())
				util::fail(fError(), u8"Targets must be constructed and not out of scope");
			if (&target.sink() != this)
				util::fail(fError(), u8"Target [", target.toString(), u8"] must originate from sink");
		}
	}

	/* add the instruction to the interface */
	pInterface->addInst(inst);
}
