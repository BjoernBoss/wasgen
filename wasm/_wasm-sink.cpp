#include "_wasm-sink.h"
#include "_wasm-module.h"

#include "../util/logging.h"

wasm::_Sink::_Sink(const wasm::_Function& function) {
	/* validate that the function can be used as sink-target */
	if (!function.valid())
		util::fail(u8"Functions must be constructed to create a sink to them");
	if (!function.imported().name.empty())
		util::fail(u8"Sinks cannot be created for imported function [", function.id(), u8"]");
	wasm::_Prototype prototype = function.prototype();

	/* check if the module is closed or the function has already been bound */
	wasm::_Module& module = wasm::_Function{ function }.module();
	module.fCheckClosed();
	if (module.pFunction.list[function.index()].bound)
		util::fail(u8"Sink cannot be created for function [", function.id(), u8"] for which a sink has already been created before");
	module.pFunction.list[function.index()].bound = true;

	/* setup the sink-state */
	pFunction = function;
	if (prototype.valid()) {
		const auto& params = prototype.parameter();
		for (size_t i = 0; i < params.size(); ++i) {
			pVariables.list.push_back({ {}, params[i].type });
			if (!params[i].id.empty())
				pVariables.list.back().name = *pVariables.names.insert(params[i].id).first;
		}
	}
	pParameter = uint32_t(pVariables.list.size());
	module.pFunction.list[function.index()].sink = this;

	/* setup the sink-interface */
	pInterface = module.pInterface->sink(pFunction);
}
wasm::_Sink::~_Sink() {
	fClose();
}

void wasm::_Sink::fClose() {
	if (pClosed)
		return;
	pClosed = true;

	/* close all remaining scopes and unregister the sink from the function */
	fPopUntil(0);
	pFunction.module().pFunction.list[pFunction.index()].sink = 0;

	/* mark the sink as closed */
	pInterface->close(*this);
}
void wasm::_Sink::fCheckClosed() const {
	if (pClosed)
		util::fail(u8"Cannot change the closed sink to function [", pFunction.id(), u8"]");
}
void wasm::_Sink::fPopUntil(uint32_t size) {
	while (pTargets.size() > size) {
		pTargets.pop_back();
		pInterface->popScope();
	}
}
bool wasm::_Sink::fCheckTarget(uint32_t index, size_t stamp, bool soft) const {
	if (index < pTargets.size() && pTargets[index].stamp == stamp)
		return true;
	if (!soft)
		util::fail(u8"Target [", index, u8"] is out of scope for sink to function [", pFunction.id(), u8"]");
	return false;
}
void wasm::_Sink::fSetupTarget(const wasm::_Prototype& prototype, std::u8string_view label, wasm::_ScopeType type, wasm::_Target& target) {
	fCheckClosed();

	/* validate the prototype */
	if (prototype.valid() && &prototype.module() != &pFunction.module())
		util::fail(u8"Prototype [", prototype.id(), u8"] must originate from same module as sink for sink to function [", pFunction.id(), u8"]");

	/* no need to validate the uniqueness of the label, as the name can be duplicated */
	detail::TargetState state = { prototype, std::u8string{ label }, ++pNextStamp, type, false };
	pTargets.push_back(std::move(state));
	uint32_t index = uint32_t(pTargets.size() - 1);

	/* configure the target */
	target.pIndex = index;
	target.pStamp = pNextStamp;

	/* notify the interface about the added scope */
	pInterface->pushScope(target);
}
void wasm::_Sink::fToggleTarget(uint32_t index, size_t stamp) {
	/* ignore the target if its already out of scope or already toggled */
	if (index >= pTargets.size() || pTargets[index].stamp != stamp)
		return;
	if (pTargets[index].type != wasm::_ScopeType::conditional || pTargets[index].otherwise)
		return;

	/* pop all intermediate objects and toggle the target */
	fPopUntil(index + 1);
	pTargets[index].otherwise = true;

	/* notify the interface about the changed scope */
	pInterface->toggleConditional();
}
void wasm::_Sink::fCloseTarget(uint32_t index, size_t stamp) {
	/* ignore the target if its already out of scope */
	if (index < pTargets.size() && pTargets[index].stamp == stamp)
		fPopUntil(index);
}

wasm::_Variable wasm::_Sink::parameter(uint32_t index) {
	if (index >= pParameter)
		util::fail(u8"Index [", index, u8"] out of bounds for sink to function [", pFunction.id(), u8"]");
	return wasm::_Variable{ *this, index };
}
wasm::_Variable wasm::_Sink::local(wasm::_Type type, std::u8string_view name) {
	fCheckClosed();

	/* validate the name */
	std::u8string _name{ name };
	if (!_name.empty() && pVariables.names.contains(_name))
		util::fail(u8"Variable with name [", _name, u8"] already defined in sink to function [", pFunction.id(), u8"]");

	/* setup the variable-state */
	detail::VariableState state = { {}, type };

	/* allocate the next id and register the next variable */
	if (!_name.empty())
		state.name = *pVariables.names.insert(_name).first;
	pVariables.list.push_back(std::move(state));
	wasm::_Variable variable{ *this, uint32_t(pVariables.list.size() - 1) };

	/* notify the interface about the added variable */
	pInterface->addLocal(variable);
	return variable;
}
wasm::_Function wasm::_Sink::function() const {
	return pFunction;
}
void wasm::_Sink::close() {
	fClose();
}

wasm::_List<wasm::_Variable, wasm::_Sink::_LocalList> wasm::_Sink::locals() const {
	return { _LocalList{ const_cast<wasm::_Sink*>(this) } };
}

void wasm::_Sink::operator[](const wasm::_InstConst& inst) {
	fCheckClosed();
	pInterface->addInst(inst);
}
void wasm::_Sink::operator[](const wasm::_InstSimple& inst) {
	fCheckClosed();
	pInterface->addInst(inst);
}
void wasm::_Sink::operator[](const wasm::_InstMemory& inst) {
	fCheckClosed();

	/* validate the instruction-operands */
	if (inst.memory.valid() && &inst.memory.module() != &pFunction.module())
		util::fail(u8"Memory [", inst.memory.id(), u8"] must originate from same module as sink for sink to function [", pFunction.id(), u8"]");
	if (inst.type == wasm::_InstMemory::Type::copy) {
		if (inst.source.valid() && &inst.source.module() != &pFunction.module())
			util::fail(u8"Memory [", inst.source.id(), u8"] must originate from same module as sink for sink to function [", pFunction.id(), u8"]");
	}

	/* add the instruction to the interface */
	pInterface->addInst(inst);
}
void wasm::_Sink::operator[](const wasm::_InstTable& inst) {
	fCheckClosed();

	/* validate the instruction-operands */
	if (inst.table.valid() && &inst.table.module() != &pFunction.module())
		util::fail(u8"Table [", inst.table.id(), u8"] must originate from same module as sink for sink to function [", pFunction.id(), u8"]");
	if (inst.type == wasm::_InstTable::Type::copy) {
		if (inst.source.valid() && &inst.source.module() != &pFunction.module())
			util::fail(u8"Table [", inst.source.id(), u8"] must originate from same module as sink for sink to function [", pFunction.id(), u8"]");
	}

	/* add the instruction to the interface */
	pInterface->addInst(inst);
}
void wasm::_Sink::operator[](const wasm::_InstLocal& inst) {
	fCheckClosed();

	/* validate the instruction-operands */
	if (!inst.variable.valid())
		util::fail(u8"Locals must be constructed for sink to function [", pFunction.id(), u8"]");
	if (&inst.variable.sink() != this)
		util::fail(u8"Local [", inst.variable.name(), u8"] must originate from sink to function [", pFunction.id(), u8"]");

	/* add the instruction to the interface */
	pInterface->addInst(inst);
}
void wasm::_Sink::operator[](const wasm::_InstGlobal& inst) {
	fCheckClosed();

	/* validate the instruction-operands */
	if (!inst.global.valid())
		util::fail(u8"Globals must be constructed for sink to function [", pFunction.id(), u8"]");
	if (&inst.global.module() != &pFunction.module())
		util::fail(u8"Global [", inst.global.id(), u8"] must originate from same module as sink for sink to function [", pFunction.id(), u8"]");

	/* add the instruction to the interface */
	pInterface->addInst(inst);
}
void wasm::_Sink::operator[](const wasm::_InstFunction& inst) {
	fCheckClosed();

	/* validate the instruction-operands */
	if (!inst.function.valid())
		util::fail(u8"Functions must be constructed for sink to function [", pFunction.id(), u8"]");
	if (&inst.function.module() != &pFunction.module())
		util::fail(u8"Function [", inst.function.id(), u8"] must originate from same module as sink for sink to function [", pFunction.id(), u8"]");

	/* add the instruction to the interface */
	pInterface->addInst(inst);
}
void wasm::_Sink::operator[](const wasm::_InstIndirect& inst) {
	fCheckClosed();

	/* validate the instruction-operands */
	if (inst.table.valid() && &inst.table.module() != &pFunction.module())
		util::fail(u8"Table [", inst.table.id(), u8"] must originate from same module as sink for sink to function [", pFunction.id(), u8"]");
	if (inst.prototype.valid() && &inst.prototype.module() != &pFunction.module())
		util::fail(u8"Prototype [", inst.prototype.id(), u8"] must originate from same module as sink for sink to function [", pFunction.id(), u8"]");

	/* add the instruction to the interface */
	pInterface->addInst(inst);
}
void wasm::_Sink::operator[](const wasm::_InstBranch& inst) {
	fCheckClosed();

	/* validate the instruction-operands */
	if (!inst.target.valid())
		util::fail(u8"Targets must be constructed and not out of scope for sink to function [", pFunction.id(), u8"]");
	if (&inst.target.sink() != this)
		util::fail(u8"Target [", inst.target.label(), u8"] must originate from sink to function [", pFunction.id(), u8"]");
	if (inst.type == wasm::_InstBranch::Type::table) {
		for (size_t i = 0; i < inst.list.size(); ++i) {
			const wasm::_Target& target = inst.list.begin()[i];

			if (!target.valid())
				util::fail(u8"Targets must be constructed and not out of scope for sink to function [", pFunction.id(), u8"]");
			if (&target.sink() != this)
				util::fail(u8"Target [", target.label(), u8"] must originate from sink to function [", pFunction.id(), u8"]");

		}
	}

	/* add the instruction to the interface */
	pInterface->addInst(inst);
}
