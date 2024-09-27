#include "_wasm-sink.h"
#include "_wasm-module.h"

wasm::_Target::_Target(wasm::_Sink& sink) : SinkMember{ sink, 0 } {}

void wasm::_Target::fSetup(const wasm::_Prototype& prototype, std::u8string_view label, detail::TargetType type) {
	pIndex = pSink->fSetupTarget(prototype, label, type);
	pStamp = fGet()->stamp;
}
void wasm::_Target::fToggle() {
	pSink->fToggleTarget(pIndex, pStamp);
}
void wasm::_Target::fClose() {
	pSink->fCloseTarget(pIndex, pStamp);
}

bool wasm::_Target::valid() const {
	return (pSink != 0 && pSink->fCheckTarget(pIndex, pStamp, true));
}
uint32_t wasm::_Target::index() const {
	pSink->fCheckTarget(pIndex, pStamp, false);
	return uint32_t(pSink->pTargets.size() - pIndex);
}
std::u8string_view wasm::_Target::label() const {
	pSink->fCheckTarget(pIndex, pStamp, false);
	return fGet()->label;
}
wasm::_Prototype wasm::_Target::prototype() const {
	pSink->fCheckTarget(pIndex, pStamp, false);
	return fGet()->prototype;
}


wasm::_IfThen::_IfThen(wasm::_Sink& sink, const wasm::_Prototype& prototype, std::u8string_view label) : _Target{ sink } {
	fSetup(prototype, label, detail::TargetType::then);
}
wasm::_IfThen::~_IfThen() {
	fClose();
}
void wasm::_IfThen::close() {
	fClose();
}
void wasm::_IfThen::otherwise() {
	fToggle();
}


wasm::_Loop::_Loop(wasm::_Sink& sink, const wasm::_Prototype& prototype, std::u8string_view label) : _Target{ sink } {
	fSetup(prototype, label, detail::TargetType::loop);
}
wasm::_Loop::~_Loop() {
	fClose();
}
void wasm::_Loop::close() {
	fClose();
}

wasm::_Block::_Block(wasm::_Sink& sink, const wasm::_Prototype& prototype, std::u8string_view label) : _Target{ sink } {
	fSetup(prototype, label, detail::TargetType::block);
}
wasm::_Block::~_Block() {
	fClose();
}
void wasm::_Block::close() {
	fClose();
}
