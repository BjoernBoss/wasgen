#include "text-sink.h"
#include "text-module.h"

writer::text::Sink::Sink(text::Module* module, std::u8string header) {
	pModule = module;
	pLocals = std::move(header);
	pDepth = u8"\n    ";
}

void writer::text::Sink::fAddLine(const std::u8string_view& str) {
	str::BuildTo(pBody, pDepth, str);
}
void writer::text::Sink::fPush(const std::u8string_view& name) {
	str::BuildTo(pBody, pDepth, u8'(', name);
	pDepth.append(u8"  ");
}
void writer::text::Sink::fPop() {
	pDepth.resize(pDepth.size() - 2);
	str::BuildTo(pBody, pDepth, u8')');
}

void writer::text::Sink::pushScope(const wasm::Target& target) {
	std::u8string text;

	/* fetch the block-type */
	if (target.type() == wasm::ScopeType::conditional)
		text = u8"if";
	else if (target.type() == wasm::ScopeType::loop)
		text = u8"loop";
	else
		text = u8"block";

	/* add the label and the prototype */
	text.append(text::MakeId(target.label()));
	text.append(text::MakePrototype(target.prototype()));

	/* push the actual block out */
	fPush(text);
	if (target.type() == wasm::ScopeType::conditional)
		fPush(u8"then");
}
void writer::text::Sink::popScope(wasm::ScopeType type) {
	fPop();
	if (type == wasm::ScopeType::conditional)
		fPop();
}
void writer::text::Sink::toggleConditional() {
	fPop();
	fPush(u8"else");
}
void writer::text::Sink::close(const wasm::Sink& sink) {
	str::BuildTo(pModule->pDefined, pLocals, pBody, u8"\n  )");
	delete this;
}
void writer::text::Sink::addLocal(const wasm::Variable& local) {
	str::BuildTo(pLocals,
		u8"\n    (local",
		text::MakeId(local.name()),
		text::MakeType(local.type()),
		u8')');
}
void writer::text::Sink::addInst(const wasm::InstConst& inst) {
	fAddLine(u8"wasm::InstConst");
}
void writer::text::Sink::addInst(const wasm::InstSimple& inst) {
	fAddLine(u8"wasm::InstSimple");
}
void writer::text::Sink::addInst(const wasm::InstMemory& inst) {
	fAddLine(u8"wasm::InstMemory");
}
void writer::text::Sink::addInst(const wasm::InstTable& inst) {
	fAddLine(u8"wasm::InstTable");
}
void writer::text::Sink::addInst(const wasm::InstLocal& inst) {
	fAddLine(u8"wasm::InstLocal");
}
void writer::text::Sink::addInst(const wasm::InstGlobal& inst) {
	fAddLine(u8"wasm::InstGlobal");
}
void writer::text::Sink::addInst(const wasm::InstFunction& inst) {
	fAddLine(u8"wasm::InstFunction");
}
void writer::text::Sink::addInst(const wasm::InstIndirect& inst) {
	fAddLine(u8"wasm::InstIndirect");
}
void writer::text::Sink::addInst(const wasm::InstBranch& inst) {
	fAddLine(u8"wasm::InstBranch");
}
