/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2024-2026 Bjoern Boss Henrichsen */
#include "split-module.h"
#include "split-sink.h"

wasm::split::Sink::Sink(std::vector<wasm::SinkInterface*>&& childs) : pSinks{ std::move(childs) } {}

void wasm::split::Sink::pushScope(const wasm::Target& target) {
	for (auto& child : pSinks)
		child->pushScope(target);
}
void wasm::split::Sink::popScope(wasm::ScopeType type) {
	for (auto& child : pSinks)
		child->popScope(type);
}
void wasm::split::Sink::toggleConditional() {
	for (auto& child : pSinks)
		child->toggleConditional();
}
void wasm::split::Sink::close(const wasm::Sink& sink) {
	for (auto& child : pSinks)
		child->close(sink);
}
void wasm::split::Sink::addLocal(const wasm::Variable& local) {
	for (auto& child : pSinks)
		child->addLocal(local);
}
void wasm::split::Sink::addComment(std::u8string_view text) {
	for (auto& child : pSinks)
		child->addComment(text);
}
void wasm::split::Sink::addInst(const wasm::InstSimple& inst) {
	for (auto& child : pSinks)
		child->addInst(inst);
}
void wasm::split::Sink::addInst(const wasm::InstConst& inst) {
	for (auto& child : pSinks)
		child->addInst(inst);
}
void wasm::split::Sink::addInst(const wasm::InstOperand& inst) {
	for (auto& child : pSinks)
		child->addInst(inst);
}
void wasm::split::Sink::addInst(const wasm::InstWidth& inst) {
	for (auto& child : pSinks)
		child->addInst(inst);
}
void wasm::split::Sink::addInst(const wasm::InstMemory& inst) {
	for (auto& child : pSinks)
		child->addInst(inst);
}
void wasm::split::Sink::addInst(const wasm::InstTable& inst) {
	for (auto& child : pSinks)
		child->addInst(inst);
}
void wasm::split::Sink::addInst(const wasm::InstLocal& inst) {
	for (auto& child : pSinks)
		child->addInst(inst);
}
void wasm::split::Sink::addInst(const wasm::InstGlobal& inst) {
	for (auto& child : pSinks)
		child->addInst(inst);
}
void wasm::split::Sink::addInst(const wasm::InstFunction& inst) {
	for (auto& child : pSinks)
		child->addInst(inst);
}
void wasm::split::Sink::addInst(const wasm::InstIndirect& inst) {
	for (auto& child : pSinks)
		child->addInst(inst);
}
void wasm::split::Sink::addInst(const wasm::InstBranch& inst) {
	for (auto& child : pSinks)
		child->addInst(inst);
}
