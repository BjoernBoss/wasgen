#include "_wasm-sink.h"

#include "_wasm-module.h"

#include "../util/logging.h"

void wasm::_Sink::fCloseUntil(uint32_t size) {
	while (pTargets.size() > size)
		pTargets.pop_back();
}
bool wasm::_Sink::fCheckTarget(uint32_t index, size_t stamp, bool soft) const {
	if (index < pTargets.size() && pTargets[index].stamp == stamp)
		return true;
	if (!soft)
		util::fail(u8"Target [", index, u8"] is out of scope for sink to function [", pFunction.id(), u8"]");
	return false;
}
uint32_t wasm::_Sink::fSetupTarget(const wasm::_Prototype& prototype, std::u8string_view label, detail::TargetType type) {
	/* validate the prototype */
	if (prototype.valid() && &prototype.module() != &pFunction.module())
		util::fail(u8"Prototype [", prototype.id(), u8"] must originate from same module as sink for sink to function [", pFunction.id(), u8"]");

	/* no need to validate the uniqueness of the label, as the name can be duplicated */
	detail::TargetState state = { prototype, std::u8string{ label }, ++pNextStamp, type };
	pTargets.push_back(std::move(state));
	return uint32_t(pTargets.size() - 1);
}
void wasm::_Sink::fToggleTarget(uint32_t index, size_t stamp) {
	/* ignore the target if its already out of scope or already toggled */
	if (index >= pTargets.size() || pTargets[index].stamp != stamp)
		return;
	if (pTargets[index].type != detail::TargetType::then)
		return;

	/* pop all intermediate objects and toggle the target */
	fCloseUntil(index + 1);
	pTargets[index].type = detail::TargetType::otherwise;
}
void wasm::_Sink::fCloseTarget(uint32_t index, size_t stamp) {
	/* ignore the target if its already out of scope */
	if (index < pTargets.size() && pTargets[index].stamp == stamp)
		fCloseUntil(index);
}

wasm::_Variable wasm::_Sink::parameter(uint32_t index) {
	if (index >= pParameter)
		util::fail(u8"Index [", index, u8"] out of bounds for sink to function [", pFunction.id(), u8"]");
	return wasm::_Variable{ *this, index };
}
wasm::_Variable wasm::_Sink::local(wasm::_Type type, std::u8string_view name) {
	std::u8string _name{ name };

	/* validate the name */
	if (!_name.empty() && pVariables.names.contains(_name))
		util::fail(u8"Variable with name [", _name, u8"] already defined in sink to function [", pFunction.id(), u8"]");

	/* setup the variable-state */
	detail::VariableState state = { {}, type };

	/* allocate the next id and register the next variable */
	if (!_name.empty())
		state.name = *pVariables.names.insert(_name).first;
	pVariables.list.push_back(std::move(state));
	return wasm::_Variable{ *this, uint32_t(pVariables.list.size() - 1) };
}
wasm::_Function wasm::_Sink::function() const {
	return pFunction;
}
