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
		{ std::declval<typename Type::Sink>().popTarget() };
		{ std::declval<typename Type::Sink>().popThen() };
		{ std::declval<typename Type::Sink>().popElse() };
		{ std::declval<typename Type::Sink>().toggleElse() };
		{ std::declval<typename Type::Sink>().getParameter(i) } -> std::same_as<typename Type::Variable>;
		{ std::declval<typename Type::Sink>().addLocal(wasm::Type::i32, id) } -> std::same_as<typename Type::Variable>;
		{ std::declval<typename Type::Sink>().addInst(std::declval<const typename Type::Instruction&>()) };
		{ std::declval<typename Type::Sink>().pushTarget(id, std::declval<const typename Type::Prototype*>(), true) } -> std::same_as<typename Type::Target>;
		{ std::declval<typename Type::Sink>().pushConditional(std::declval<const typename Type::Prototype*>()) };
	};

	template <wasm::IsSink Writer> struct Sink;

	namespace detail {
		struct Pushed {
			size_t stamp = 0;
			size_t index = 0;
		};

		template <class Writer>
		struct JumpBase : public wasm::Target<Writer> {
		protected:
			wasm::Sink<Writer>& pSink;
			detail::Pushed pPushed;
			constexpr JumpBase(wasm::Sink<Writer>& sink, const std::u8string_view& label, const wasm::Prototype<Writer>* prototype, bool loop) : wasm::Target<Writer>{ std::move(sink.fOpen(pPushed, label, prototype, loop)) }, pSink{ sink } {}
			constexpr void fClose() {
				pSink.fClose(pPushed);
			}
			constexpr bool fValid() const {
				return pSink.fCheck(pPushed);
			}
		};
	}

	/* if-then block of a conditional (will automatically end the instruction-block at destruction and can be toggled to else-block) */
	template <class Writer>
	struct IfThen {
	private:
		wasm::Sink<Writer>& pSink;
		detail::Pushed pPushed;

	public:
		constexpr IfThen(wasm::Sink<Writer>& sink) : pSink{ sink } {
			pSink.fOpen(pPushed, 0);
		}
		constexpr IfThen(wasm::Sink<Writer>& sink, const wasm::Prototype<Writer>& prototype) : pSink{ sink } {
			pSink.fOpen(pPushed, &prototype);
		}
		IfThen(wasm::IfThen<Writer>&&) = delete;
		IfThen(const wasm::IfThen<Writer>&) = delete;
		constexpr ~IfThen() {
			pSink.fClose(pPushed);
		}

	public:
		constexpr bool valid() const {
			return pSink.fCheck(pPushed);
		}
		constexpr void close() {
			pSink.fClose(pPushed);
		}
		constexpr void otherwise() {
			pSink.fToggle(pPushed);
		}
	};

	/* loop block, which can act as wasm::Target (will automatically end the instruction-block at destruction) */
	template <class Writer>
	struct Loop : public detail::JumpBase<Writer> {
		constexpr Loop(wasm::Sink<Writer>& sink, const std::u8string_view& label = {}) : detail::JumpBase<Writer>{ sink, label, 0, true } {}
		constexpr Loop(wasm::Sink<Writer>& sink, const wasm::Prototype<Writer>& prototype, const std::u8string_view& label = {}) : detail::JumpBase<Writer>{ sink, label, &prototype, true } {}
		constexpr ~Loop() {
			detail::JumpBase<Writer>::fClose();
		}
		constexpr bool valid() const {
			return detail::JumpBase<Writer>::fCheck();
		}
		constexpr void close() {
			detail::JumpBase<Writer>::fClose();
		}
	};

	/* jump block, which can act as wasm::Target (will automatically end the instruction-block at destruction) */
	template <class Writer>
	struct Block : public detail::JumpBase<Writer> {
		constexpr Block(wasm::Sink<Writer>& sink, const std::u8string_view& label = {}) : detail::JumpBase<Writer>{ sink, label, 0, false } {}
		constexpr Block(wasm::Sink<Writer>& sink, const wasm::Prototype<Writer>& prototype, const std::u8string_view& label = {}) : detail::JumpBase<Writer>{ sink, label, &prototype, false } {}
		constexpr ~Block() {
			detail::JumpBase<Writer>::fClose();
		}
		constexpr bool valid() const {
			return detail::JumpBase<Writer>::fCheck();
		}
		constexpr void close() {
			detail::JumpBase<Writer>::fClose();
		}
	};

	/* instruction-sink to collect instructions for a function [instantiated via wasm::Module] */
	template <wasm::IsSink Writer>
	struct Sink {
		friend struct wasm::IfThen<Writer>;
		friend struct wasm::Target<Writer>;
	private:
		struct Opened {
			size_t stamp = 0;
			enum class Type : uint8_t {
				target,
				then,
				otherwise
			} type = Type::target;
		};

	private:
		/* stack must be empty at destruction, as all openable objects cannot be copied or moved, can only be
		*	constructed by this object, and must therefore be destructed before this object is destructed */
		std::vector<Opened> pStack;
		size_t pNext = 0;

	public:
		Writer::Sink self;

	private:
		constexpr void fCloseUntil(size_t count) {
			while (pStack.size() > count) {
				if (pStack.back().type == Opened::Type::target)
					self.popTarget();
				else if (pStack.back().type == Opened::Type::then)
					self.popThen();
				else
					self.popElse();
				pStack.pop_back();
			}
		}
		constexpr bool fCheck(const detail::Pushed& pushed) const {
			return (pushed.index < pStack.size() && pStack[pushed.index].stamp == pushed.stamp);
		}
		constexpr void fClose(const detail::Pushed& pushed) {
			if (fCheck(pushed))
				fCloseUntil(pushed.index);
		}
		constexpr void fToggle(const detail::Pushed& pushed) {
			if (fCheck(pushed) && pStack[pushed.index].type == Opened::Type::then) {
				fCloseUntil(pushed.index + 1);
				pStack.back().type = Opened::Type::otherwise;
				self.toggleElse();
			}
		}
		constexpr void fOpen(detail::Pushed& pushed, const wasm::Prototype<Writer>* prototype) {
			pushed.stamp = pNext;
			pushed.index = pStack.size();
			self.pushConditional(prototype == 0 ? 0 : &prototype->self);
			pStack.emplace_back(++pNext, Opened::Type::then);
		}
		constexpr Writer::Target fOpen(detail::Pushed& pushed, const std::u8string_view& label, const wasm::Prototype<Writer>* prototype, bool loop) {
			pushed.stamp = pNext;
			pushed.index = pStack.size();
			typename Writer::Target base = self.pushJump(label, (prototype == 0 ? 0 : &prototype->self), loop);
			pStack.emplace_back(++pNext, Opened::Type::target);
			return base;
		}

	public:
		constexpr Sink(Writer::Sink&& self) : self{ std::move(self) } {}
		Sink(const wasm::Sink<Writer>&) = delete;
		constexpr Sink(wasm::Sink<Writer>&&) = default;

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
	};
}
