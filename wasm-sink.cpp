#include "wasm-sink.h"
#include "wasm-module.h"

wasm::Sink::Sink(const wasm::Function& function) {
	/* validate that the function can be used as sink-target */
	if (!function.valid())
		throw wasm::Exception{ L"Functions must be constructed to create a sink to them" };
	if (function.imported())
		throw wasm::Exception{ L"Sinks cannot be created for imported function [", function.toString(), L"]" };
	pModule = &function.module();

	/* check if the module is closed or the function has already been bound */
	pModule->fCheckClosed();
	if (pModule->pFunction.list[function.index()].bound)
		throw wasm::Exception{ L"Sink cannot be created for function [", function.toString(), L"] for which a sink has already been created before" };
	pModule->pFunction.list[function.index()].bound = true;

	/* setup the sink-state */
	pFunction = function;
	const auto& params = function.prototype().parameter();
	for (size_t i = 0; i < params.size(); ++i)
		pVariables.list.push_back({ {}, params[i].type });
	pParameter = uint32_t(pVariables.list.size());
	pModule->pFunction.list[function.index()].sink = this;

	/* setup the sink-interface */
	pInterface = pModule->pInterface->sink(pFunction);
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
	pModule->pFunction.list[pFunction.index()].sink = 0;

	/* mark the sink as closed */
	pInterface->close(*this);
}
void wasm::Sink::fCheckClosed() const {
	if (pClosed)
		throw wasm::Exception{ fError(), L"Cannot change the closed" };
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
		throw wasm::Exception{ fError(), L"Target [", index, L"] is out of scope" };
	return false;
}
void wasm::Sink::fSetupValidTarget(const wasm::Prototype& prototype, std::u8string_view id, wasm::ScopeType type, wasm::Target& target) {
	/* validate the prototype */
	if (!prototype.valid())
		throw wasm::Exception{ fError(), L"Prototype must be constructed" };
	if (&prototype.module() != pModule)
		throw wasm::Exception{ fError(), L"Prototype [", prototype.toString(), L"] must originate from same module as function" };

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
void wasm::Sink::fSetupTarget(const wasm::Prototype& prototype, std::u8string_view id, wasm::ScopeType type, wasm::Target& target) {
	fCheckClosed();
	fSetupValidTarget(prototype, id, type, target);
}
void wasm::Sink::fSetupTarget(std::initializer_list<wasm::Type> params, std::initializer_list<wasm::Type> result, std::u8string_view id, wasm::ScopeType type, wasm::Target& target) {
	fCheckClosed();
	fSetupValidTarget(pModule->prototype(params, result), id, type, target);
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
	/* validate the parameter-index */
	if (index >= pParameter)
		throw wasm::Exception{ fError(), L"Parameter index [", index, L"] out of bounds" };
	return wasm::Variable{ *this, index };
}
wasm::Variable wasm::Sink::local(wasm::Type type, std::u8string_view id) {
	fCheckClosed();

	/* validate the id */
	std::u8string _id{ id };
	if (!_id.empty() && pVariables.ids.contains(_id))
		throw wasm::Exception{ fError(), L"Variable [", _id, L"] already defined in sink" };

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

void wasm::Sink::operator[](const wasm::InstSimple& inst) {
	fCheckClosed();
	pInterface->addInst(inst);
}
void wasm::Sink::operator[](const wasm::InstConst& inst) {
	fCheckClosed();
	pInterface->addInst(inst);
}
void wasm::Sink::operator[](const wasm::InstOperand& inst) {
	fCheckClosed();
	pInterface->addInst(inst);
}
void wasm::Sink::operator[](const wasm::InstWidth& inst) {
	fCheckClosed();
	pInterface->addInst(inst);
}
void wasm::Sink::operator[](const wasm::InstMemory& inst) {
	fCheckClosed();

	/* validate the instruction-operands */
	if (!inst.memory.valid())
		throw wasm::Exception{ fError(), L"Memories must be constructed" };
	if (&inst.memory.module() != pModule)
		throw wasm::Exception{ fError(), L"Memory [", inst.memory.toString(), L"] must originate from same module as function" };
	if (inst.type == wasm::InstMemory::Type::copy) {
		if (!inst.destination.valid())
			throw wasm::Exception{ fError(), L"Memories must be constructed" };
		if (&inst.destination.module() != pModule)
			throw wasm::Exception{ fError(), L"Memory [", inst.destination.toString(), L"] must originate from same module as function" };
	}

	/* add the instruction to the interface */
	pInterface->addInst(inst);
}
void wasm::Sink::operator[](const wasm::InstTable& inst) {
	fCheckClosed();

	/* validate the instruction-operands */
	if (!inst.table.valid())
		throw wasm::Exception{ fError(), L"Tables must be constructed" };
	if (&inst.table.module() != pModule)
		throw wasm::Exception{ fError(), L"Table [", inst.table.toString(), L"] must originate from same module as function" };
	if (inst.type == wasm::InstTable::Type::copy) {
		if (!inst.destination.valid())
			throw wasm::Exception{ fError(), L"Tables must be constructed" };
		if (&inst.destination.module() != pModule)
			throw wasm::Exception{ fError(), L"Table [", inst.destination.toString(), L"] must originate from same module as function" };
	}

	/* add the instruction to the interface */
	pInterface->addInst(inst);
}
void wasm::Sink::operator[](const wasm::InstLocal& inst) {
	fCheckClosed();

	/* validate the instruction-operands */
	if (!inst.variable.valid())
		throw wasm::Exception{ fError(), L"Locals must be constructed" };
	if (&inst.variable.sink() != this)
		throw wasm::Exception{ fError(), L"Local [", inst.variable.toString(), L"] must originate from sink" };

	/* add the instruction to the interface */
	pInterface->addInst(inst);
}
void wasm::Sink::operator[](const wasm::InstGlobal& inst) {
	fCheckClosed();

	/* validate the instruction-operands */
	if (!inst.global.valid())
		throw wasm::Exception{ fError(), L"Globals must be constructed" };
	if (&inst.global.module() != pModule)
		throw wasm::Exception{ fError(), L"Global [", inst.global.toString(), L"] must originate from same module as function" };

	/* add the instruction to the interface */
	pInterface->addInst(inst);
}
void wasm::Sink::operator[](const wasm::InstFunction& inst) {
	fCheckClosed();

	/* validate the instruction-operands */
	if (!inst.function.valid())
		throw wasm::Exception{ fError(), L"Functions must be constructed" };
	if (&inst.function.module() != pModule)
		throw wasm::Exception{ fError(), L"Function [", inst.function.toString(), L"] must originate from same module as function" };

	/* add the instruction to the interface */
	pInterface->addInst(inst);
}
void wasm::Sink::operator[](const wasm::InstIndirect& inst) {
	fCheckClosed();

	/* validate the instruction-operands */
	if (!inst.table.valid())
		throw wasm::Exception{ fError(), L"Tables must be constructed" };
	if (&inst.table.module() != pModule)
		throw wasm::Exception{ fError(), L"Table [", inst.table.toString(), L"] must originate from same module as function" };
	if (!inst.prototype.valid())
		throw wasm::Exception{ fError(), L"Prototype must be constructed" };
	if (&inst.prototype.module() != pModule)
		throw wasm::Exception{ fError(), L"Prototype [", inst.prototype.toString(), L"] must originate from same module as function" };

	/* add the instruction to the interface */
	pInterface->addInst(inst);
}
void wasm::Sink::operator[](const wasm::InstBranch& inst) {
	fCheckClosed();

	/* validate the instruction-operands */
	if (!inst.target.valid())
		throw wasm::Exception{ fError(), L"Targets must be constructed and not out of scope" };
	if (&inst.target.sink() != this)
		throw wasm::Exception{ fError(), L"Target [", inst.target.toString(), L"] must originate from sink" };
	if (inst.type == wasm::InstBranch::Type::table) {
		for (size_t i = 0; i < inst.list.size(); ++i) {
			const wasm::Target& target = inst.list.begin()[i];

			if (!target.valid())
				throw wasm::Exception{ fError(), L"Targets must be constructed and not out of scope" };
			if (&target.sink() != this)
				throw wasm::Exception{ fError(), L"Target [", target.toString(), L"] must originate from sink" };
		}
	}

	/* add the instruction to the interface */
	pInterface->addInst(inst);
}
