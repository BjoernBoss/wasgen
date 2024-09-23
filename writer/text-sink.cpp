#include "text-sink.h"

writer::text::Target::Target(text::SinkImpl& sink, size_t stamp, size_t index) : sink{ sink }, stamp{ stamp }, index{ index } {}
std::u8string writer::text::Target::toString() const {
	return sink.targetString(*this);
}


void writer::text::SinkImpl::fCheck(size_t instance, bool checkIfClosed) const {
	if ((instance != pInstance) || (checkIfClosed && pClosed))
		util::fail(u8"Cannot perform operations on closed sink");
}
std::unordered_set<std::u8string>::iterator writer::text::SinkImpl::fAllocLabel(std::u8string label) {
	if (label.empty())
		return pLabels.end();

	/* check if the label is already in use */
	if (pLabels.count(label) != 0)
		util::fail(u8"Label with name [", label, "] already defined");

	/* allocate the new label */
	return pLabels.emplace(std::move(label)).first;
}
void writer::text::SinkImpl::fCloseUntil(size_t size) {
	while (pPushed.size() > size) {
		/* close the topmost object */
		fPop();
		if (pPushed.back().type != Pushed::Type::target)
			fPop();

		/* cleanup the pushed object */
		if (pPushed.back().label != pLabels.end())
			pLabels.erase(pPushed.back().label);
		pPushed.pop_back();
	}
}
void writer::text::SinkImpl::fPush(const std::u8string_view& name) {
	str::BuildTo(pBody, pDepth, u8'(', name);
	pDepth.append(u8"  ");
}
void writer::text::SinkImpl::fPop() {
	pDepth.resize(pDepth.size() - 2);
	str::BuildTo(pBody, pDepth, u8')');
}
void writer::text::SinkImpl::fAddLine(const std::u8string_view& str) {
	str::BuildTo(pBody, pDepth, str);
}

std::u8string writer::text::SinkImpl::targetString(const text::Target& target) {
	if (target.index >= pPushed.size() || target.stamp != pPushed[target.index].stamp)
		util::fail(u8"Target cannot be used anymore as its referencing block has already been closed");
	if (pPushed[target.index].label != pLabels.end())
		return *pPushed[target.index].label;
	return str::Int<std::u8string>(pPushed.size() - target.index);
}
size_t writer::text::SinkImpl::nextInstance(std::u8string content, std::vector<wasm::Param> params, std::u8string& out) {
	/* finalize the last instance */
	fCloseUntil(0);
	if (!pOutput.empty())
		str::BuildTo(out, pOutput, pBody, u8"\n  )");

	/* allocate the next instance (leave the stamps untouched in order to invalidate old targets) */
	pParameter = std::move(params);
	pOutput = std::move(content);
	pDepth = u8"\n    ";
	pLocals.clear();
	pBody.clear();
	pLocalIndex = uint32_t(pParameter.size());
	return ++pInstance;
}

writer::text::Target writer::text::SinkImpl::pushConditional(size_t instance, const std::u8string_view& label, const text::Prototype* prototype) {
	fCheck(instance, true);
	std::unordered_set<std::u8string>::iterator it = fAllocLabel(std::u8string(label));

	/* construct the description line and push it as well as the initial then line */
	std::u8string text = u8"if";
	if (prototype != 0)
		str::BuildTo(text, text::MakeId(label), u8" (type ", prototype->toString(), u8')');
	fPush(text);
	fPush(u8"then");

	/* allocate the next pushed stamp */
	text::Target out{ *this, ++pStamp, pPushed.size() };
	pPushed.emplace_back(it, pStamp, Pushed::Type::then);
	return out;
}
writer::text::Target writer::text::SinkImpl::pushTarget(size_t instance, const std::u8string_view& label, const text::Prototype* prototype, bool block) {
	fCheck(instance, true);
	std::unordered_set<std::u8string>::iterator it = fAllocLabel(std::u8string(label));

	/* construct the description line and push it */
	std::u8string text = (block ? u8"block" : u8"loop");
	if (prototype != 0)
		str::BuildTo(text, text::MakeId(label), u8" (type ", prototype->toString(), u8')');
	fPush(text);

	/* allocate the next pushed stamp */
	text::Target out{ *this, ++pStamp, pPushed.size() };
	pPushed.emplace_back(it, pStamp, Pushed::Type::target);
	return out;
}

void writer::text::SinkImpl::toggleConditional(text::Target& target) {
	/* no need for instance-check, as target stamps will implicitly perform the same checks */
	if (target.index >= pPushed.size() || pPushed[target.index].stamp != target.stamp)
		return;
	if (pPushed[target.index].type != Pushed::Type::then)
		return;

	/* pop all intermediate objects and open the new else-block */
	fCloseUntil(target.index + 1);
	fPop();
	fPush(u8"else");
	pPushed[target.index].type = Pushed::Type::otherwise;
}
void writer::text::SinkImpl::pop(text::Target& target) {
	/* no need for instance-check, as target stamps will implicitly perform the same checks */
	if (target.index < pPushed.size() && pPushed[target.index].stamp == target.stamp)
		fCloseUntil(target.index);
}

writer::text::Variable writer::text::SinkImpl::getParameter(size_t instance, uint32_t index) {
	fCheck(instance, true);
	if (index >= pParameter.size())
		util::fail(u8"Parameter index [", index, u8"] is out of bounds");

	/* setup the new parameter-string */
	std::u8string id;
	if (!pParameter[index].id.empty())
		id = str::Build<std::u8string>(u8"$", pParameter[index].id);
	return text::Variable{ id, index };
}
writer::text::Variable writer::text::SinkImpl::addLocal(size_t instance, wasm::Type type, const std::u8string_view& id) {
	fCheck(instance, true);

	/* validate the id and register it */
	std::u8string _id{ id };
	if (!_id.empty()) {
		if (pLocals.count(_id) != 0)
			util::fail(u8"Local variable with id [", _id, "] already defined");
		pLocals.emplace(_id);
		_id = str::Build<std::u8string>(u8"$", _id);
	}

	/* add the local to the output and construct the variable */
	str::BuildTo(pOutput, u8"\n    (local", text::MakeId(id), text::MakeType(type), u8')');
	return text::Variable{ std::move(_id), pLocalIndex++ };
}
void writer::text::SinkImpl::addInst(size_t instance, const text::Instruction& inst) {
	fCheck(instance, true);
	fAddLine(inst.string);
}
void writer::text::SinkImpl::close(size_t instance) {
	fCheck(instance, false);
	if (!pClosed)
		fCloseUntil(0);
	pClosed = true;
}


writer::text::SinkWrapper::SinkWrapper(text::SinkImpl* sink, size_t instance) : pSink{ sink }, pInstance{ instance } {}
writer::text::SinkWrapper::SinkWrapper(text::SinkWrapper&& sink) noexcept : pSink{ sink.pSink }, pInstance{ sink.pInstance } {
	sink.pSink = 0;
	sink.pInstance = 0;
}

writer::text::Target writer::text::SinkWrapper::pushLoop(const std::u8string_view& label, const text::Prototype* prototype) {
	return pSink->pushTarget(pInstance, label, prototype, false);
}
void writer::text::SinkWrapper::popLoop(text::Target& target) {
	pSink->pop(target);
}

writer::text::Target writer::text::SinkWrapper::pushBlock(const std::u8string_view& label, const text::Prototype* prototype) {
	return pSink->pushTarget(pInstance, label, prototype, true);
}
void writer::text::SinkWrapper::popBlock(text::Target& target) {
	pSink->pop(target);
}

writer::text::Target writer::text::SinkWrapper::pushConditional(const std::u8string_view& label, const text::Prototype* prototype) {
	return pSink->pushConditional(pInstance, label, prototype);
}
void writer::text::SinkWrapper::toggleConditional(text::Target& target) {
	pSink->toggleConditional(target);
}
void writer::text::SinkWrapper::popConditional(text::Target& target) {
	pSink->pop(target);
}

writer::text::Variable writer::text::SinkWrapper::getParameter(uint32_t index) {
	return pSink->getParameter(pInstance, index);
}
writer::text::Variable writer::text::SinkWrapper::addLocal(wasm::Type type, const std::u8string_view& id) {
	return pSink->addLocal(pInstance, type, id);
}
void writer::text::SinkWrapper::addInst(const text::Instruction& inst) {
	return pSink->addInst(pInstance, inst);
}
void writer::text::SinkWrapper::close() {
	return pSink->close(pInstance);
}
