#include "wasm-sink.h"
#include "wasm-module.h"

wasm::Target::Target(wasm::Sink& sink) : SinkMember{ sink, 0 } {}

void wasm::Target::fSetup(const wasm::Prototype& prototype, std::u8string_view label, wasm::ScopeType type) {
	pSink->fSetupTarget(prototype, label, type, *this);
}
void wasm::Target::fToggle() {
	pSink->fToggleTarget(pIndex, pStamp);
}
void wasm::Target::fClose() {
	pSink->fCloseTarget(pIndex, pStamp);
}

bool wasm::Target::valid() const {
	return (pSink != 0 && pSink->fCheckTarget(pIndex, pStamp, true));
}
uint32_t wasm::Target::index() const {
	pSink->fCheckTarget(pIndex, pStamp, false);
	return uint32_t(pSink->pTargets.size() - pIndex);
}
std::u8string_view wasm::Target::label() const {
	pSink->fCheckTarget(pIndex, pStamp, false);
	return fGet()->label;
}
wasm::Prototype wasm::Target::prototype() const {
	pSink->fCheckTarget(pIndex, pStamp, false);
	return fGet()->prototype;
}
wasm::ScopeType wasm::Target::type() const {
	pSink->fCheckTarget(pIndex, pStamp, false);
	return fGet()->type;
}


wasm::IfThen::IfThen(wasm::Sink& sink, std::u8string_view label, const wasm::Prototype& prototype) : Target{ sink } {
	fSetup(prototype, label, wasm::ScopeType::conditional);
}
wasm::IfThen::~IfThen() {
	fClose();
}
void wasm::IfThen::close() {
	fClose();
}
void wasm::IfThen::otherwise() {
	fToggle();
}


wasm::Loop::Loop(wasm::Sink& sink, std::u8string_view label, const wasm::Prototype& prototype) : Target{ sink } {
	fSetup(prototype, label, wasm::ScopeType::loop);
}
wasm::Loop::~Loop() {
	fClose();
}
void wasm::Loop::close() {
	fClose();
}


wasm::Block::Block(wasm::Sink& sink, std::u8string_view label, const wasm::Prototype& prototype) : Target{ sink } {
	fSetup(prototype, label, wasm::ScopeType::block);
}
wasm::Block::~Block() {
	fClose();
}
void wasm::Block::close() {
	fClose();
}
