/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2024 Bjoern Boss Henrichsen */
#include "wasm-sink.h"
#include "../objects/wasm-module.h"

wasm::Target::Target() {
	pSink = 0;
}
wasm::Target::Target(wasm::Target&& target) noexcept {
	pIndex = target.pIndex;
	pStamp = target.pStamp;
	pSink = target.pSink;
	target.pSink = 0;
}
wasm::Target& wasm::Target::operator=(wasm::Target&& target) noexcept {
	pIndex = target.pIndex;
	pStamp = target.pStamp;
	pSink = target.pSink;
	target.pSink = 0;
	return *this;
}
wasm::Target::Target(wasm::Sink& sink) : SinkMember{ sink, 0 } {}
wasm::Target::~Target() {
	try {
		fClose();
	}
	catch (const wasm::Exception& e) {
		/* defer the exception to the sink (sink must exist as otherwise no exception will be thrown) */
		pSink->fDeferredException(e.what());
	}
}

void wasm::Target::fSetup(std::u8string_view label, const wasm::Prototype& prototype, wasm::ScopeType type) {
	pSink->fSetupTarget(prototype, label, type, *this);
}
void wasm::Target::fSetup(std::u8string_view label, std::vector<wasm::Type> params, std::vector<wasm::Type> result, wasm::ScopeType type) {
	pSink->fSetupTarget(params, result, label, type, *this);
}
void wasm::Target::fToggle() {
	pSink->fToggleTarget(pIndex, pStamp);
}
void wasm::Target::fClose() {
	if (pSink != 0)
		pSink->fCloseTarget(pIndex, pStamp);
	pSink = 0;
}

void wasm::Target::close() {
	fClose();
}
bool wasm::Target::valid() const {
	if (pSink == 0)
		return false;
	return pSink->fCheckTarget(pIndex, pStamp, true);
}
uint32_t wasm::Target::index() const {
	pSink->fCheckTarget(pIndex, pStamp, false);
	return uint32_t(pSink->pTargets.size() - pIndex - 1);
}
std::u8string_view wasm::Target::id() const {
	pSink->fCheckTarget(pIndex, pStamp, false);
	return fGet()->id;
}
wasm::Prototype wasm::Target::prototype() const {
	pSink->fCheckTarget(pIndex, pStamp, false);
	return fGet()->prototype;
}
wasm::ScopeType wasm::Target::type() const {
	pSink->fCheckTarget(pIndex, pStamp, false);
	return fGet()->type;
}
std::u8string wasm::Target::toString() const {
	pSink->fCheckTarget(pIndex, pStamp, false);
	std::u8string_view id = fGet()->id;
	if (!id.empty())
		return str::u8::Build(u8"$", id);
	return str::u8::Build(uint32_t(pSink->pTargets.size() - pIndex - 1));
}


wasm::IfThen::IfThen(wasm::Sink& sink, std::u8string_view label, const wasm::Prototype& prototype) : Target{ sink } {
	fSetup(label, prototype, wasm::ScopeType::conditional);
}
wasm::IfThen::IfThen(wasm::Sink& sink, std::u8string_view label, std::vector<wasm::Type> params, std::vector<wasm::Type> result) : Target{ sink } {
	fSetup(label, params, result, wasm::ScopeType::conditional);
}
wasm::IfThen::IfThen(wasm::Sink* sink, std::u8string_view label, const wasm::Prototype& prototype) : Target{ *sink } {
	fSetup(label, prototype, wasm::ScopeType::conditional);
}
wasm::IfThen::IfThen(wasm::Sink* sink, std::u8string_view label, std::vector<wasm::Type> params, std::vector<wasm::Type> result) : Target{ *sink } {
	fSetup(label, params, result, wasm::ScopeType::conditional);
}
void wasm::IfThen::otherwise() {
	fToggle();
}


wasm::Loop::Loop(wasm::Sink& sink, std::u8string_view label, const wasm::Prototype& prototype) : Target{ sink } {
	fSetup(label, prototype, wasm::ScopeType::loop);
}
wasm::Loop::Loop(wasm::Sink& sink, std::u8string_view label, std::vector<wasm::Type> params, std::vector<wasm::Type> result) : Target{ sink } {
	fSetup(label, params, result, wasm::ScopeType::loop);
}
wasm::Loop::Loop(wasm::Sink* sink, std::u8string_view label, const wasm::Prototype& prototype) : Target{ *sink } {
	fSetup(label, prototype, wasm::ScopeType::loop);
}
wasm::Loop::Loop(wasm::Sink* sink, std::u8string_view label, std::vector<wasm::Type> params, std::vector<wasm::Type> result) : Target{ *sink } {
	fSetup(label, params, result, wasm::ScopeType::loop);
}


wasm::Block::Block(wasm::Sink& sink, std::u8string_view label, const wasm::Prototype& prototype) : Target{ sink } {
	fSetup(label, prototype, wasm::ScopeType::block);
}
wasm::Block::Block(wasm::Sink& sink, std::u8string_view label, std::vector<wasm::Type> params, std::vector<wasm::Type> result) : Target{ sink } {
	fSetup(label, params, result, wasm::ScopeType::block);
}
wasm::Block::Block(wasm::Sink* sink, std::u8string_view label, const wasm::Prototype& prototype) : Target{ *sink } {
	fSetup(label, prototype, wasm::ScopeType::block);
}
wasm::Block::Block(wasm::Sink* sink, std::u8string_view label, std::vector<wasm::Type> params, std::vector<wasm::Type> result) : Target{ *sink } {
	fSetup(label, params, result, wasm::ScopeType::block);
}
