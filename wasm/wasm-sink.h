#pragma once

#include "wasm-common.h"
#include "wasm-types.h"

namespace wasm {
	/* writer must provide type Sink, which the wasm::Sink internally instantiates
	*	must implement various interaction member-functions for it
	*	pushed jump-bases/conditionals
	*		guaranteed to only be closed once per function object and in stacked order
	*		for [then] blocks, they will guaranteed at most be toggled once before being closed */
	template <class Type>
	concept IsSink = requires(const std::u8string_view id, uint32_t i) {
		typename Type::Sink;
		{ std::declval<typename Type::Sink>().pushLoop(id, std::declval<const typename Type::Prototype*>()) } -> std::same_as<typename Type::Target>;
		{ std::declval<typename Type::Sink>().popLoop(std::declval<typename Type::Target&>()) };
		{ std::declval<typename Type::Sink>().pushBlock(id, std::declval<const typename Type::Prototype*>()) } -> std::same_as<typename Type::Target>;
		{ std::declval<typename Type::Sink>().popBlock(std::declval<typename Type::Target&>()) };
		{ std::declval<typename Type::Sink>().pushConditional(id, std::declval<const typename Type::Prototype*>()) } -> std::same_as<typename Type::Target>;
		{ std::declval<typename Type::Sink>().toggleConditional(std::declval<typename Type::Target&>()) };
		{ std::declval<typename Type::Sink>().popConditional(std::declval<typename Type::Target&>()) };
		{ std::declval<typename Type::Sink>().getParameter(i) } -> std::same_as<typename Type::Variable>;
		{ std::declval<typename Type::Sink>().addLocal(wasm::Type::i32, id) } -> std::same_as<typename Type::Variable>;
		{ std::declval<typename Type::Sink>().addInst(std::declval<const typename Type::Instruction&>()) };
		{ std::declval<typename Type::Sink>().close() };
	};

	template <wasm::IsSink Writer> struct Sink;

	/* if-then block of a conditional (will automatically end the instruction-block at destruction and can be toggled to else-block) */
	template <wasm::IsSink Writer>
	struct IfThen : public wasm::Target<Writer> {
	private:
		typename Writer::Sink& pSink;

	public:
		constexpr IfThen(wasm::Sink<Writer>& sink, const std::u8string_view& label = {}) : wasm::Target<Writer>{ std::move(sink.self.pushConditional(label, 0)) }, pSink{ sink.self } {}
		constexpr IfThen(wasm::Sink<Writer>& sink, const wasm::Prototype<Writer>& prototype, const std::u8string_view& label = {}) : wasm::Target<Writer>{ std::move(sink.self.pushConditional(label, &prototype.self)) }, pSink{ sink.self } {}
		constexpr ~IfThen() {
			pSink.popConditional(wasm::Target<Writer>::self);
		}
		constexpr void close() {
			pSink.popConditional(wasm::Target<Writer>::self);
		}
		constexpr void otherwise() {
			pSink.toggleConditional(wasm::Target<Writer>::self);
		}
	};

	/* loop block, which can act as wasm::Target (will automatically end the instruction-block at destruction) */
	template <wasm::IsSink Writer>
	struct Loop : public wasm::Target<Writer> {
	private:
		typename Writer::Sink& pSink;

	public:
		constexpr Loop(wasm::Sink<Writer>& sink, const std::u8string_view& label = {}) : wasm::Target<Writer>{ std::move(sink.self.pushLoop(label, 0)) }, pSink{ sink.self } {}
		constexpr Loop(wasm::Sink<Writer>& sink, const wasm::Prototype<Writer>& prototype, const std::u8string_view& label = {}) : wasm::Target<Writer>{ std::move(sink.self.pushLoop(label, &prototype.self)) }, pSink{ sink.self } {}
		constexpr ~Loop() {
			pSink.popLoop(wasm::Target<Writer>::self);
		}
		constexpr void close() {
			pSink.popLoop(wasm::Target<Writer>::self);
		}
	};

	/* jump block, which can act as wasm::Target (will automatically end the instruction-block at destruction) */
	template <wasm::IsSink Writer>
	struct Block : public wasm::Target<Writer> {
	private:
		typename Writer::Sink& pSink;

	public:
		constexpr Block(wasm::Sink<Writer>& sink, const std::u8string_view& label = {}) : wasm::Target<Writer>{ std::move(sink.self.pushBlock(label, 0)) }, pSink{ sink.self } {}
		constexpr Block(wasm::Sink<Writer>& sink, const wasm::Prototype<Writer>& prototype, const std::u8string_view& label = {}) : wasm::Target<Writer>{ std::move(sink.self.pushBlock(label, &prototype.self)) }, pSink{ sink.self } {}
		constexpr ~Block() {
			pSink.popBlock(wasm::Target<Writer>::self);
		}
		constexpr void close() {
			pSink.popBlock(wasm::Target<Writer>::self);
		}
	};

	/* instruction-sink to collect instructions for a function [instantiated via wasm::Module]
	*	Important: No produced/bound types must outlive the wasm::Sink object */
	template <wasm::IsSink Writer>
	struct Sink {
		Writer::Sink self;

	public:
		constexpr Sink(Writer::Sink&& self) : self{ std::move(self) } {}
		Sink(const wasm::Sink<Writer>&) = delete;
		constexpr Sink(wasm::Sink<Writer>&&) = default;
		constexpr ~Sink() {
			self.close();
		}

	public:
		constexpr wasm::Variable<Writer> parameter(uint32_t index) {
			return wasm::Variable<Writer>{ std::move(self.getParameter(index)) };
		}
		constexpr wasm::Variable<Writer> local(wasm::Type type, const std::u8string_view& id = {}) {
			return wasm::Variable<Writer>{ std::move(self.addLocal(type, id)) };
		}
		constexpr void operator[](const wasm::Instruction<Writer>& inst) {
			self.addInst(inst.self);
		}
		constexpr void close() {
			self.close();
		}
	};
}
