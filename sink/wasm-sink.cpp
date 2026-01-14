/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2024-2026 Bjoern Boss Henrichsen */
#include "wasm-sink.h"
#include "../objects/wasm-module.h"

wasm::Sink::Sink(const wasm::Function& function) {
	/* validate that the function can be used as sink-target */
	if (!function.valid())
		throw wasm::Exception{ "Functions must be constructed to create a sink to them" };
	if (function.imported())
		throw wasm::Exception{ "Sinks cannot be created for imported function [", function.toString(), ']' };
	pModule = &function.module();

	/* check if the module is closed or the function has already been bound */
	pModule->fCheck();
	if (pModule->pFunction.list[function.index()].bound)
		throw wasm::Exception{ "Sink cannot be created for function [", function.toString(), "] for which a sink has already been created before" };
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
	try {
		fClose();
	}
	catch (const wasm::Exception& e) {
		/* defer the exception to the module */
		pModule->fDeferredException(e);
	}
}

wasm::Type wasm::Sink::fMapOperand(wasm::OpType operand) const {
	switch (operand) {
	case wasm::OpType::i32:
		return wasm::Type::i32;
	case wasm::OpType::i64:
		return wasm::Type::i64;
	case wasm::OpType::f32:
		return wasm::Type::f32;
	case wasm::OpType::f64:
		return wasm::Type::f64;
	default:
		throw wasm::Exception{ "Unknown wasm::OpType type [", size_t(operand), "] encountered" };
	}
}
std::u8string wasm::Sink::fError() const {
	return str::u8::Build(u8"Error in sink to function [", pFunction.toString(), u8"]: ");
}
void wasm::Sink::fCheck() const {
	/* check if any queued exceptions need to be thrown */
	if (!pException.empty()) {
		std::string err;
		std::swap(err, pException);
		throw wasm::Exception{ err };
	}

	/* check if the sink has already been closed */
	if (pClosed)
		throw wasm::Exception{ fError(), "Cannot change the closed" };
}
void wasm::Sink::fClose() {
	if (pClosed)
		return;

	/* process any queued exceptions and afterwards mark the sink
	*	as closed (otherwise checking will throw an exception) */
	fCheck();
	pClosed = true;

	/* close all remaining scopes and unregister the sink from the function */
	fPopUntil(0);
	pModule->pFunction.list[pFunction.index()].sink = 0;

	/* perform the type checking */
	if (!fScope().unreachable) {
		fPopTypes(pFunction.prototype(), false);
		fCheckEmpty();
	}

	/* mark the sink as closed */
	pInterface->close(*this);
}
void wasm::Sink::fDeferredException(const wasm::Exception& error) {
	if (pException.empty())
		pException = error.what();
}
wasm::Variable wasm::Sink::fParam(uint32_t index) {
	/* validate the parameter-index */
	if (index >= pParameter)
		throw wasm::Exception{ fError(), "Parameter index [", index, "] out of bounds" };
	return wasm::Variable{ *this, index };
}

void wasm::Sink::fPopUntil(uint32_t size) {
	while (pTargets.size() > size) {
		/* perform the type checking (only if this is not a cleanup after a potential exception) */
		if (!pTargets.back().scope.unreachable) {
			fPopTypes(pTargets.back().state.prototype, false);
			fCheckEmpty();
		}
		else
			pStack.resize(pTargets.back().scope.stack);
		fPushTypes(pTargets.back().state.prototype, false);

		/* notify the interface about the removed target and remove it */
		pInterface->popScope(pTargets.back().state.type);
		pTargets.pop_back();
	}
}
bool wasm::Sink::fCheckTarget(uint32_t index, size_t stamp, bool soft) const {
	if (index < pTargets.size() && pTargets[index].state.stamp == stamp)
		return true;
	if (!soft)
		throw wasm::Exception{ fError(), "Target [", index, "] is out of scope" };
	return false;
}
void wasm::Sink::fSetupValidTarget(const wasm::Prototype& prototype, std::u8string_view id, wasm::ScopeType type, wasm::Target& target) {
	/* validate the prototype */
	if (!prototype.valid())
		throw wasm::Exception{ fError(), "Prototype must be constructed" };
	if (&prototype.module() != pModule)
		throw wasm::Exception{ fError(), "Prototype [", prototype.toString(), "] must originate from same module as function" };

	/* perform the type checking */
	if (type == wasm::ScopeType::conditional)
		fPopTypes({ wasm::Type::i32 });
	fPopTypes(prototype, true);
	fPushTypes(prototype, true);

	/* no need to validate the uniqueness of the id, as the name can be duplicated */
	detail::TargetState state = { prototype, std::u8string{ id }, ++pNextStamp, type, false };
	Scope scope = { pStack.size() - prototype.parameter().size(), fScope().unreachable };
	pTargets.push_back({ std::move(state), scope });
	uint32_t index = uint32_t(pTargets.size() - 1);

	/* configure the target */
	target.pIndex = index;
	target.pStamp = pNextStamp;

	/* notify the interface about the added scope */
	pInterface->pushScope(target);
}
void wasm::Sink::fSetupTarget(const wasm::Prototype& prototype, std::u8string_view id, wasm::ScopeType type, wasm::Target& target) {
	fCheck();
	fSetupValidTarget(prototype, id, type, target);
}
void wasm::Sink::fSetupTarget(std::vector<wasm::Type> params, std::vector<wasm::Type> result, std::u8string_view id, wasm::ScopeType type, wasm::Target& target) {
	fCheck();
	fSetupValidTarget(pModule->prototype(params, result), id, type, target);
}
void wasm::Sink::fToggleTarget(uint32_t index, size_t stamp) {
	/* ignore the target if its already out of scope or already toggled */
	if (index >= pTargets.size() || pTargets[index].state.stamp != stamp)
		return;
	if (pTargets[index].state.type != wasm::ScopeType::conditional || pTargets[index].state.otherwise)
		return;

	/* pop all intermediate objects and toggle the target */
	fPopUntil(index + 1);
	pTargets.back().state.otherwise = true;

	/* perform the type checking (i.e. the closed block returned all expected parameter) and restore the state */
	if (!pTargets.back().scope.unreachable) {
		fPopTypes(pTargets.back().state.prototype, false);
		fCheckEmpty();
	}
	pTargets.back().scope.unreachable = false;
	pStack.resize(pTargets.back().scope.stack);
	fPushTypes(pTargets.back().state.prototype, true);

	/* notify the interface about the changed scope */
	pInterface->toggleConditional();
}
void wasm::Sink::fCloseTarget(uint32_t index, size_t stamp) {
	/* ignore the target if its already out of scope */
	if (index < pTargets.size() && pTargets[index].state.stamp == stamp)
		fPopUntil(index);
}

void wasm::Sink::fTypesFailed(std::string_view expected, std::string_view found) const {
	if (!fScope().unreachable)
		throw wasm::Exception{ fError(), "Expected [", expected, "] but found [", found, ']' };
}
void wasm::Sink::fPopFailed(size_t count, std::string_view expected) const {
	const Scope& scope = fScope();

	/* setup the description of the found types */
	size_t available = pStack.size() - scope.stack;
	size_t start = scope.stack + (count > available ? 0 : (available - count));
	std::string found = fMakeTypeList(pStack.begin() + start, pStack.end(), [](auto& t) { return t; });
	fTypesFailed(expected, found);
}
void wasm::Sink::fCheckEmpty() const {
	const Scope& scope = fScope();
	if (pStack.size() > scope.stack)
		fPopFailed(pStack.size() - scope.stack, "");
}
const wasm::Sink::Scope& wasm::Sink::fScope() const {
	if (pTargets.empty())
		return pRoot;
	return pTargets.back().scope;
}
wasm::Sink::Scope& wasm::Sink::fScope() {
	if (pTargets.empty())
		return pRoot;
	return pTargets.back().scope;
}
void wasm::Sink::fPopTypes(std::initializer_list<wasm::Type> types) {
	Scope& scope = fScope();
	if (scope.unreachable)
		return;

	/* validate that the given types exist and pop them */
	if (pStack.size() - scope.stack >= types.size() && std::equal(pStack.end() - types.size(), pStack.end(), types.begin())) {
		pStack.resize(pStack.size() - types.size());
		return;
	}

	/* setup the description of the expected types */
	std::string expected = fMakeTypeList(types.begin(), types.end(), [](auto& t) { return t; });
	fPopFailed(types.size(), expected);
}
void wasm::Sink::fPopTypes(const wasm::Prototype& prototype, bool params) {
	Scope& scope = fScope();
	if (scope.unreachable)
		return;

	/* validate that the given types exist and pop them */
	std::string expected;
	size_t count = 0;
	if (params) {
		const auto& list = prototype.parameter();

		/* check for a match */
		if (pStack.size() - scope.stack >= list.size() && std::equal(pStack.end() - list.size(), pStack.end(), list.begin(),
			[](wasm::Type l, const wasm::Param& r) { return (l == r.type); })) {
			pStack.resize(pStack.size() - list.size());
			return;
		}

		/* setup the description of the expected types */
		expected = fMakeTypeList(list.begin(), list.end(), [](auto& p) { return p.type; });
		count = list.size();
	}
	else {
		const auto& list = prototype.result();

		/* check for a match */
		if (pStack.size() - scope.stack >= list.size() && std::equal(pStack.end() - list.size(), pStack.end(), list.begin())) {
			pStack.resize(pStack.size() - list.size());
			return;
		}

		/* setup the description of the expected types */
		expected = fMakeTypeList(list.begin(), list.end(), [](auto& t) { return t; });
		count = list.size();
	}
	fPopFailed(count, expected);
}
void wasm::Sink::fSwapTypes(std::initializer_list<wasm::Type> pop, std::initializer_list<wasm::Type> push) {
	fPopTypes(pop);
	fPushTypes(push);
}
void wasm::Sink::fPushTypes(std::initializer_list<wasm::Type> types) {
	pStack.insert(pStack.end(), types.begin(), types.end());
}
void wasm::Sink::fPushTypes(const wasm::Prototype& prototype, bool params) {
	if (params) for (size_t i = 0; i < prototype.parameter().size(); ++i)
		pStack.push_back(prototype.parameter()[i].type);
	else
		pStack.insert(pStack.end(), prototype.result().begin(), prototype.result().end());
}

wasm::Variable wasm::Sink::param(uint32_t index) {
	return fParam(index);
}
wasm::Variable wasm::Sink::local(wasm::Type type, std::u8string_view id) {
	fCheck();

	/* validate the id */
	std::u8string _id{ id };
	if (!_id.empty() && pVariables.ids.contains(_id))
		throw wasm::Exception{ fError(), "Variable [", _id, "] already defined in sink" };

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
void wasm::Sink::comment(std::u8string_view text) {
	fCheck();
	pInterface->addComment(text);
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
	fCheck();

	/* perform the type checking */
	switch (inst.type) {
	case wasm::InstSimple::Type::drop:
		if (pStack.size() - fScope().stack < 1)
			fPopFailed(1, "any");
		else
			fPopTypes({ pStack.back() });
		break;
	case wasm::InstSimple::Type::ret:
		fPopTypes(pFunction.prototype(), false);
		fScope().unreachable = true;
		break;
	case wasm::InstSimple::Type::select:
		if (pStack.size() - fScope().stack < 3)
			fPopFailed(3, "opt1, opt2, i32");
		else {
			wasm::Type type = pStack.end()[-2];
			fSwapTypes({ type, type, wasm::Type::i32 }, { type });
		}
		break;
	case wasm::InstSimple::Type::selectRefFunction:
		fSwapTypes({ wasm::Type::refFunction, wasm::Type::refFunction, wasm::Type::i32 }, { wasm::Type::refFunction });
		break;
	case wasm::InstSimple::Type::selectRefExtern:
		fSwapTypes({ wasm::Type::refExtern, wasm::Type::refExtern, wasm::Type::i32 }, { wasm::Type::refExtern });
		break;
	case wasm::InstSimple::Type::refTestNull:
		if (pStack.size() - fScope().stack < 1 || (pStack.back() != wasm::Type::refExtern && pStack.back() != wasm::Type::refFunction))
			fPopFailed(1, "ref");
		else {
			fSwapTypes({ pStack.back() }, { wasm::Type::i32 });
		}
		break;
	case wasm::InstSimple::Type::refNullFunction:
		fPushTypes({ wasm::Type::refFunction });
		break;
	case wasm::InstSimple::Type::refNullExtern:
		fPushTypes({ wasm::Type::refExtern });
		break;
	case wasm::InstSimple::Type::expandIntSigned:
	case wasm::InstSimple::Type::expandIntUnsigned:
		fSwapTypes({ wasm::Type::i32 }, { wasm::Type::i64 });
		break;
	case wasm::InstSimple::Type::shrinkInt:
		fSwapTypes({ wasm::Type::i64 }, { wasm::Type::i32 });
		break;
	case wasm::InstSimple::Type::expandFloat:
		fSwapTypes({ wasm::Type::f32 }, { wasm::Type::f64 });
		break;
	case wasm::InstSimple::Type::shrinkFloat:
		fSwapTypes({ wasm::Type::f64 }, { wasm::Type::f32 });
		break;
	case wasm::InstSimple::Type::unreachable:
		fScope().unreachable = true;
		break;
	case wasm::InstSimple::Type::nop:
		break;
	default:
		throw wasm::Exception{ "Unknown wasm::InstSimple type [", size_t(inst.type), "] encountered" };
	}

	/* add the instruction to the interface */
	pInterface->addInst(inst);
}
void wasm::Sink::operator[](const wasm::InstConst& inst) {
	fCheck();

	/* perform the type checking */
	if (std::holds_alternative<uint32_t>(inst.value))
		fPushTypes({ wasm::Type::i32 });
	else if (std::holds_alternative<uint64_t>(inst.value))
		fPushTypes({ wasm::Type::i64 });
	else if (std::holds_alternative<float>(inst.value))
		fPushTypes({ wasm::Type::f32 });
	else if (std::holds_alternative<double>(inst.value))
		fPushTypes({ wasm::Type::f64 });
	else
		throw wasm::Exception{ "Unknown wasm::InstConst type encountered" };

	/* add the instruction to the interface */
	pInterface->addInst(inst);
}
void wasm::Sink::operator[](const wasm::InstOperand& inst) {
	fCheck();

	/* perform the type checking */
	wasm::Type type = fMapOperand(inst.operand);
	if (inst.type == wasm::InstOperand::Type::equal || inst.type == wasm::InstOperand::Type::notEqual)
		fSwapTypes({ type, type }, { wasm::Type::i32 });
	else
		fSwapTypes({ type, type }, { type });

	/* add the instruction to the interface */
	pInterface->addInst(inst);
}
void wasm::Sink::operator[](const wasm::InstWidth& inst) {
	fCheck();

	/* perform the type checking */
	wasm::Type itype = (inst.width32 ? wasm::Type::i32 : wasm::Type::i64), ftype = (inst.width32 ? wasm::Type::f32 : wasm::Type::f64);
	switch (inst.type) {
	case wasm::InstWidth::Type::equalZero:
		fSwapTypes({ itype }, { wasm::Type::i32 });
		break;
	case wasm::InstWidth::Type::greater:
	case wasm::InstWidth::Type::less:
	case wasm::InstWidth::Type::greaterEqual:
	case wasm::InstWidth::Type::lessEqual:
		fSwapTypes({ ftype, ftype }, { wasm::Type::i32 });
		break;
	case wasm::InstWidth::Type::greaterSigned:
	case wasm::InstWidth::Type::greaterUnsigned:
	case wasm::InstWidth::Type::lessSigned:
	case wasm::InstWidth::Type::lessUnsigned:
	case wasm::InstWidth::Type::greaterEqualSigned:
	case wasm::InstWidth::Type::greaterEqualUnsigned:
	case wasm::InstWidth::Type::lessEqualSigned:
	case wasm::InstWidth::Type::lessEqualUnsigned:
		fSwapTypes({ itype, itype }, { wasm::Type::i32 });
		break;
	case wasm::InstWidth::Type::divSigned:
	case wasm::InstWidth::Type::divUnsigned:
	case wasm::InstWidth::Type::modSigned:
	case wasm::InstWidth::Type::modUnsigned:
	case wasm::InstWidth::Type::bitAnd:
	case wasm::InstWidth::Type::bitOr:
	case wasm::InstWidth::Type::bitXOr:
	case wasm::InstWidth::Type::bitShiftLeft:
	case wasm::InstWidth::Type::bitShiftRightSigned:
	case wasm::InstWidth::Type::bitShiftRightUnsigned:
	case wasm::InstWidth::Type::bitRotateLeft:
	case wasm::InstWidth::Type::bitRotateRight:
		fSwapTypes({ itype, itype }, { itype });
		break;
	case wasm::InstWidth::Type::bitLeadingNulls:
	case wasm::InstWidth::Type::bitTrailingNulls:
	case wasm::InstWidth::Type::bitSetCount:
		fSwapTypes({ itype }, { itype });
		break;
	case wasm::InstWidth::Type::convertToF32Signed:
	case wasm::InstWidth::Type::convertToF32Unsigned:
		fSwapTypes({ itype }, { wasm::Type::f32 });
		break;
	case wasm::InstWidth::Type::convertToF64Signed:
	case wasm::InstWidth::Type::convertToF64Unsigned:
		fSwapTypes({ itype }, { wasm::Type::f64 });
		break;
	case wasm::InstWidth::Type::convertFromF32SignedTrap:
	case wasm::InstWidth::Type::convertFromF32UnsignedTrap:
	case wasm::InstWidth::Type::convertFromF32SignedNoTrap:
	case wasm::InstWidth::Type::convertFromF32UnsignedNoTrap:
		fSwapTypes({ wasm::Type::f32 }, { itype });
		break;
	case wasm::InstWidth::Type::convertFromF64SignedTrap:
	case wasm::InstWidth::Type::convertFromF64UnsignedTrap:
	case wasm::InstWidth::Type::convertFromF64SignedNoTrap:
	case wasm::InstWidth::Type::convertFromF64UnsignedNoTrap:
		fSwapTypes({ wasm::Type::f64 }, { itype });
		break;
	case wasm::InstWidth::Type::reinterpretAsFloat:
		fSwapTypes({ itype }, { ftype });
		break;
	case wasm::InstWidth::Type::reinterpretAsInt:
		fSwapTypes({ ftype }, { itype });
		break;
	case wasm::InstWidth::Type::floatDiv:
	case wasm::InstWidth::Type::floatMin:
	case wasm::InstWidth::Type::floatMax:
	case wasm::InstWidth::Type::floatCopySign:
		fSwapTypes({ ftype, ftype }, { ftype });
		break;
	case wasm::InstWidth::Type::floatFloor:
	case wasm::InstWidth::Type::floatRound:
	case wasm::InstWidth::Type::floatCeil:
	case wasm::InstWidth::Type::floatTruncate:
	case wasm::InstWidth::Type::floatAbsolute:
	case wasm::InstWidth::Type::floatNegate:
	case wasm::InstWidth::Type::floatSquareRoot:
		fSwapTypes({ ftype }, { ftype });
		break;
	default:
		throw wasm::Exception{ "Unknown wasm::InstWidth type [", size_t(inst.type), "] encountered" };
	}

	/* add the instruction to the interface */
	pInterface->addInst(inst);
}
void wasm::Sink::operator[](const wasm::InstMemory& inst) {
	fCheck();

	/* validate the instruction-operands */
	if (!inst.memory.valid())
		throw wasm::Exception{ fError(), "Memories must be constructed" };
	if (&inst.memory.module() != pModule)
		throw wasm::Exception{ fError(), "Memory [", inst.memory.toString(), "] must originate from same module as function" };
	if (inst.type == wasm::InstMemory::Type::copy) {
		if (!inst.destination.valid())
			throw wasm::Exception{ fError(), "Memories must be constructed" };
		if (&inst.destination.module() != pModule)
			throw wasm::Exception{ fError(), "Memory [", inst.destination.toString(), "] must originate from same module as function" };
	}

	/* perform the type checking */
	wasm::Type type = fMapOperand(inst.operand);
	switch (inst.type) {
	case wasm::InstMemory::Type::load:
		fSwapTypes({ wasm::Type::i32 }, { type });
		break;
	case wasm::InstMemory::Type::load8Unsigned:
		fSwapTypes({ wasm::Type::i32 }, { type });
		break;
	case wasm::InstMemory::Type::load8Signed:
		fSwapTypes({ wasm::Type::i32 }, { type });
		break;
	case wasm::InstMemory::Type::load16Unsigned:
		fSwapTypes({ wasm::Type::i32 }, { type });
		break;
	case wasm::InstMemory::Type::load16Signed:
		fSwapTypes({ wasm::Type::i32 }, { type });
		break;
	case wasm::InstMemory::Type::load32Unsigned:
		fSwapTypes({ wasm::Type::i32 }, { type });
		break;
	case wasm::InstMemory::Type::load32Signed:
		fSwapTypes({ wasm::Type::i32 }, { type });
		break;
	case wasm::InstMemory::Type::store:
		fPopTypes({ wasm::Type::i32, type });
		break;
	case wasm::InstMemory::Type::store8:
		fPopTypes({ wasm::Type::i32, type });
		break;
	case wasm::InstMemory::Type::store16:
		fPopTypes({ wasm::Type::i32, type });
		break;
	case wasm::InstMemory::Type::store32:
		fPopTypes({ wasm::Type::i32, type });
		break;
	case wasm::InstMemory::Type::size:
		fPushTypes({ wasm::Type::i32 });
		break;
	case wasm::InstMemory::Type::grow:
		fSwapTypes({ wasm::Type::i32 }, { wasm::Type::i32 });
		break;
	case wasm::InstMemory::Type::copy:
		fPopTypes({ wasm::Type::i32, wasm::Type::i32, wasm::Type::i32 });
		break;
	case wasm::InstMemory::Type::fill:
		fPopTypes({ wasm::Type::i32, wasm::Type::i32, wasm::Type::i32 });
		break;
	default:
		throw wasm::Exception{ "Unknown wasm::InstMemory type [", size_t(inst.type), "] encountered" };
	}

	/* add the instruction to the interface */
	pInterface->addInst(inst);
}
void wasm::Sink::operator[](const wasm::InstTable& inst) {
	fCheck();

	/* validate the instruction-operands */
	if (!inst.table.valid())
		throw wasm::Exception{ fError(), "Tables must be constructed" };
	if (&inst.table.module() != pModule)
		throw wasm::Exception{ fError(), "Table [", inst.table.toString(), "] must originate from same module as function" };
	if (inst.type == wasm::InstTable::Type::copy) {
		if (!inst.destination.valid())
			throw wasm::Exception{ fError(), "Tables must be constructed" };
		if (&inst.destination.module() != pModule)
			throw wasm::Exception{ fError(), "Table [", inst.destination.toString(), "] must originate from same module as function" };
	}

	/* perform the type checking */
	switch (inst.type) {
	case wasm::InstTable::Type::get:
		fSwapTypes({ wasm::Type::i32 }, { (inst.table.functions() ? wasm::Type::refFunction : wasm::Type::refExtern) });
		break;
	case wasm::InstTable::Type::set:
		fPopTypes({ wasm::Type::i32, (inst.table.functions() ? wasm::Type::refFunction : wasm::Type::refExtern) });
		break;
	case wasm::InstTable::Type::size:
		fPushTypes({ wasm::Type::i32 });
		break;
	case wasm::InstTable::Type::grow:
		fSwapTypes({ (inst.table.functions() ? wasm::Type::refFunction : wasm::Type::refExtern), wasm::Type::i32 }, { wasm::Type::i32 });
		break;
	case wasm::InstTable::Type::copy:
		fPopTypes({ wasm::Type::i32, wasm::Type::i32, wasm::Type::i32 });
		break;
	case wasm::InstTable::Type::fill:
		fPopTypes({ wasm::Type::i32, (inst.table.functions() ? wasm::Type::refFunction : wasm::Type::refExtern), wasm::Type::i32 });
		break;
	default:
		throw wasm::Exception{ "Unknown wasm::InstTable type [", size_t(inst.type), "] encountered" };
	}

	/* add the instruction to the interface */
	pInterface->addInst(inst);
}
void wasm::Sink::operator[](const wasm::InstLocal& inst) {
	fCheck();

	/* validate the instruction-operands */
	if (!inst.variable.valid())
		throw wasm::Exception{ fError(), "Locals must be constructed" };
	if (&inst.variable.sink() != this)
		throw wasm::Exception{ fError(), "Local [", inst.variable.toString(), "] must originate from sink" };

	/* perform the type checking */
	switch (inst.type) {
	case wasm::InstLocal::Type::get:
		fPushTypes({ inst.variable.type() });
		break;
	case wasm::InstLocal::Type::set:
		fPopTypes({ inst.variable.type() });
		break;
	case wasm::InstLocal::Type::tee:
		fSwapTypes({ inst.variable.type() }, { inst.variable.type() });
		break;
	default:
		throw wasm::Exception{ "Unknown wasm::InstLocal type [", size_t(inst.type), "] encountered" };
	}

	/* add the instruction to the interface */
	pInterface->addInst(inst);
}
void wasm::Sink::operator[](const wasm::InstParam& inst) {
	switch (inst.type) {
	case wasm::InstParam::Type::get:
		wasm::Sink::operator[](wasm::InstLocal{ wasm::InstLocal::Type::get, fParam(inst.index) });
		break;
	case wasm::InstParam::Type::set:
		wasm::Sink::operator[](wasm::InstLocal{ wasm::InstLocal::Type::set, fParam(inst.index) });
		break;
	case wasm::InstParam::Type::tee:
		wasm::Sink::operator[](wasm::InstLocal{ wasm::InstLocal::Type::tee, fParam(inst.index) });
		break;
	default:
		throw wasm::Exception{ "Unknown wasm::InstParam type [", size_t(inst.type), "] encountered" };
	}
}
void wasm::Sink::operator[](const wasm::InstGlobal& inst) {
	fCheck();

	/* validate the instruction-operands */
	if (!inst.global.valid())
		throw wasm::Exception{ fError(), "Globals must be constructed" };
	if (&inst.global.module() != pModule)
		throw wasm::Exception{ fError(), "Global [", inst.global.toString(), "] must originate from same module as function" };

	/* perform the type checking */
	switch (inst.type) {
	case wasm::InstGlobal::Type::get:
		fPushTypes({ inst.global.type() });
		break;
	case wasm::InstGlobal::Type::set:
		fPopTypes({ inst.global.type() });
		break;
	default:
		throw wasm::Exception{ "Unknown wasm::InstGlobal type [", size_t(inst.type), "] encountered" };
	}

	/* add the instruction to the interface */
	pInterface->addInst(inst);
}
void wasm::Sink::operator[](const wasm::InstFunction& inst) {
	fCheck();

	/* validate the instruction-operands */
	if (!inst.function.valid())
		throw wasm::Exception{ fError(), "Functions must be constructed" };
	if (&inst.function.module() != pModule)
		throw wasm::Exception{ fError(), "Function [", inst.function.toString(), "] must originate from same module as function" };

	/* perform the type checking */
	switch (inst.type) {
	case wasm::InstFunction::Type::refFunction:
		fPushTypes({ wasm::Type::refFunction });
		break;
	case wasm::InstFunction::Type::callNormal:
		fPopTypes(inst.function.prototype(), true);
		fPushTypes(inst.function.prototype(), false);
		break;
	case wasm::InstFunction::Type::callTail:
		fPopTypes(inst.function.prototype(), true);
		if (inst.function.prototype().result() != pFunction.prototype().result()) {
			const auto& expected = pFunction.prototype().result();
			const auto& found = inst.function.prototype().result();
			fTypesFailed(fMakeTypeList(expected.begin(), expected.end(), [](auto& t) { return t; }),
				fMakeTypeList(found.begin(), found.end(), [](auto& t) { return t; }));
		}
		fScope().unreachable = true;
		break;
	default:
		throw wasm::Exception{ "Unknown wasm::InstFunction type [", size_t(inst.type), "] encountered" };
	}

	/* add the instruction to the interface */
	pInterface->addInst(inst);
}
void wasm::Sink::operator[](const wasm::InstIndirect& inst) {
	fCheck();

	/* validate the instruction-operands */
	if (!inst.table.valid())
		throw wasm::Exception{ fError(), "Tables must be constructed" };
	if (&inst.table.module() != pModule)
		throw wasm::Exception{ fError(), "Table [", inst.table.toString(), "] must originate from same module as function" };
	if (!inst.prototype.valid())
		throw wasm::Exception{ fError(), "Prototype must be constructed" };
	if (&inst.prototype.module() != pModule)
		throw wasm::Exception{ fError(), "Prototype [", inst.prototype.toString(), "] must originate from same module as function" };

	/* perform the type checking */
	switch (inst.type) {
	case wasm::InstIndirect::Type::callNormal:
		fPopTypes({ wasm::Type::i32 });
		fPopTypes(inst.prototype, true);
		fPushTypes(inst.prototype, false);
		break;
	case wasm::InstIndirect::Type::callTail:
		fPopTypes({ wasm::Type::i32 });
		fPopTypes(inst.prototype, true);
		if (inst.prototype.result() != pFunction.prototype().result()) {
			const auto& expected = pFunction.prototype().result();
			const auto& found = inst.prototype.result();
			fTypesFailed(fMakeTypeList(expected.begin(), expected.end(), [](auto& t) { return t; }),
				fMakeTypeList(found.begin(), found.end(), [](auto& t) { return t; }));
		}
		fScope().unreachable = true;
		break;
	default:
		throw wasm::Exception{ "Unknown wasm::InstIndirect type [", size_t(inst.type), "] encountered" };
	}

	/* add the instruction to the interface */
	pInterface->addInst(inst);
}
void wasm::Sink::operator[](const wasm::InstBranch& inst) {
	fCheck();

	/* validate the instruction-operands */
	if (!inst.target.valid())
		throw wasm::Exception{ fError(), "Targets must be constructed and not out of scope" };
	if (&inst.target.sink() != this)
		throw wasm::Exception{ fError(), "Target [", inst.target.toString(), "] must originate from sink" };
	if (inst.type == wasm::InstBranch::Type::table) {
		for (size_t i = 0; i < inst.list.size(); ++i) {
			const wasm::Target& target = inst.list.begin()[i];

			if (!target.valid())
				throw wasm::Exception{ fError(), "Targets must be constructed and not out of scope" };
			if (&target.sink() != this)
				throw wasm::Exception{ fError(), "Target [", target.toString(), "] must originate from sink" };
		}
	}

	/* extract the state of the target */
	const detail::TargetState& state = pTargets[inst.target.pIndex].state;

	/* perform the type checking (only parameter needs to be checked) */
	switch (inst.type) {
	case wasm::InstBranch::Type::direct:
		fPopTypes(state.prototype, state.type == wasm::ScopeType::loop);
		fScope().unreachable = true;
		break;
	case wasm::InstBranch::Type::conditional:
		fPopTypes({ wasm::Type::i32 });
		fPopTypes(state.prototype, state.type == wasm::ScopeType::loop);
		fPushTypes(state.prototype, state.type == wasm::ScopeType::loop);
		break;
	case wasm::InstBranch::Type::table:
		fPopTypes({ wasm::Type::i32 });
		for (size_t i = 0; i <= inst.list.size(); ++i) {
			const detail::TargetState& temp = (i == inst.list.size() ? state : pTargets[inst.list.begin()[i].get().pIndex].state);
			fPopTypes(temp.prototype, temp.type == wasm::ScopeType::loop);
			fPushTypes(temp.prototype, temp.type == wasm::ScopeType::loop);
		}
		fScope().unreachable = true;
		break;
	default:
		throw wasm::Exception{ "Unknown wasm::InstBranch type [", size_t(inst.type), "] encountered" };
	}

	/* add the instruction to the interface */
	pInterface->addInst(inst);
}


std::u8string wasm::Variable::toString() const {
	std::u8string_view id = fGet()->id;
	if (!id.empty())
		return str::u8::Build(u8"$", id);
	return str::u8::Build(pIndex);
}
