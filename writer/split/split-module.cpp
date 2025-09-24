/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2024-2025 Bjoern Boss Henrichsen */
#include "split-module.h"
#include "split-sink.h"

wasm::split::Module::Module(std::initializer_list<wasm::ModuleInterface*> modules) : pModules{ modules } {
	std::erase(pModules, nullptr);
}

wasm::SinkInterface* wasm::split::Module::sink(const wasm::Function& function) {
	std::vector<wasm::SinkInterface*> sinks;
	for (auto& child : pModules)
		sinks.push_back(child->sink(function));
	return new split::Sink{ std::move(sinks) };
}
void wasm::split::Module::close(const wasm::Module& module) {
	for (auto& child : pModules)
		child->close(module);
}
void wasm::split::Module::addPrototype(const wasm::Prototype& prototype) {
	for (auto& child : pModules)
		child->addPrototype(prototype);
}
void wasm::split::Module::addMemory(const wasm::Memory& memory) {
	for (auto& child : pModules)
		child->addMemory(memory);
}
void wasm::split::Module::addTable(const wasm::Table& table) {
	for (auto& child : pModules)
		child->addTable(table);
}
void wasm::split::Module::addGlobal(const wasm::Global& global) {
	for (auto& child : pModules)
		child->addGlobal(global);
}
void wasm::split::Module::addFunction(const wasm::Function& function) {
	for (auto& child : pModules)
		child->addFunction(function);
}
void wasm::split::Module::setMemoryLimit(const wasm::Memory& memory) {
	for (auto& child : pModules)
		child->setMemoryLimit(memory);
}
void wasm::split::Module::setTableLimit(const wasm::Table& table) {
	for (auto& child : pModules)
		child->setTableLimit(table);
}
void wasm::split::Module::setStartup(const wasm::Function& function) {
	for (auto& child : pModules)
		child->setStartup(function);
}
void wasm::split::Module::setValue(const wasm::Global& global, const wasm::Value& value) {
	for (auto& child : pModules)
		child->setValue(global, value);
}
void wasm::split::Module::writeData(const wasm::Memory& memory, const wasm::Value& offset, const uint8_t* data, uint32_t count) {
	for (auto& child : pModules)
		child->writeData(memory, offset, data, count);
}
void wasm::split::Module::writeElements(const wasm::Table& table, const wasm::Value& offset, const wasm::Value* values, uint32_t count) {
	for (auto& child : pModules)
		child->writeElements(table, offset, values, count);
}
