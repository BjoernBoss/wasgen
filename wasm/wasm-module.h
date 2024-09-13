#pragma once

#include <ustring/str.h>
#include <cinttypes>
#include <concepts>
#include <type_traits>
#include <limits>
#include <string>
#include <vector>

namespace wasm {
	/* native types supported by wasm */
	enum class Type : uint8_t {
		i32,
		i64,
		f32,
		f64,
		v128,
		refExtern,
		refFunction
	};

	/* parameter to construct any prototype */
	struct Param {
		std::u8string id;
		wasm::Type type;
		constexpr Param(wasm::Type type, std::u8string id = u8"") : id{ id }, type{ type } {}
	};
	struct Proto {
		std::vector<wasm::Param> params;
		std::vector<wasm::Type> result;
		constexpr Proto() = default;
		constexpr Proto(std::initializer_list<wasm::Param> params, std::initializer_list<wasm::Type> result) : params{ params }, result{ result } {}
	};

	/* wrapper to define imports or exports for memories/tables/functions */
	struct Exchange {
		std::u8string module;
		std::u8string name;
	};
	struct Import : public wasm::Exchange {
		constexpr Import(std::u8string module, std::u8string name) : wasm::Exchange{ module, name } {}
	};
	struct Export : public wasm::Exchange {
		constexpr Export(std::u8string name) : wasm::Exchange{ u8"", name } {}
	};

	/* limit used by memories and tables */
	struct Limit {
		uint32_t min = 0;
		uint32_t max = 0;
		constexpr Limit(uint32_t min = 0, uint32_t max = std::numeric_limits<uint32_t>::max()) : min{ min }, max{ std::max<uint32_t>(min, max) } {}
	};

	/* writer must provide type Memory, which the wasm::Memory internally instantiates */
	template <class Type>
	concept IsMemory = requires() {
		typename Type::Memory;
	};

	/* writer must provide type Table, which the wasm::Table internally instantiates */
	template <class Type>
	concept IsTable = requires() {
		typename Type::Table;
	};

	/* writer must provide type Local, which the wasm::Local internally instantiates */
	template <class Type>
	concept IsLocal = requires() {
		typename Type::Local;
	};

	/* writer must provide type InstBase, which the wasm::Instruction internally instantiates */
	template <class Type>
	concept IsInstBase = requires() {
		typename Type::InstBase;
	};

	/* writer must provide type JumpBase, which the wasm::Target internally instantiates */
	template <class Type>
	concept IsJumpBase = requires() {
		typename Type::JumpBase;
	};

	/* writer must provide type Function, which the wasm::Function internally instantiates
	*	must implement various interaction member-functions for it
	*	pushed jump-bases/conditionals
	*		guaranteed to only be closed once per function object and in stacked order
	*		for [then] blocks, they will guaranteed at most be toggled once before being closed */
	template <class Type>
	concept IsFunction = requires(const std::u8string_view id, const wasm::Proto proto) {
		typename Type::Function;
		{ std::declval<typename Type::Function>().popTarget() };
		{ std::declval<typename Type::Function>().popThen() };
		{ std::declval<typename Type::Function>().popElse() };
		{ std::declval<typename Type::Function>().toggleElse() };
		{ std::declval<typename Type::Function>().addLocal(wasm::Type::i32, id) } -> std::same_as<typename Type::Local>;
		{ std::declval<typename Type::Function>().addInst(std::declval<const typename Type::InstBase>()) };
		{ std::declval<typename Type::Function>().pushJumpBase(id, proto, true) } -> std::same_as<typename Type::JumpBase>;
		{ std::declval<typename Type::Function>().pushConditional(proto) };
	};

	/* writer must provide type Module, which the wasm::Module internally instantiates
	*	must implement various interaction member-functions for it */
	template <class Type>
	concept IsModule = requires(Type state, const std::u8string_view id, const wasm::Limit limit, const wasm::Proto prototype, const wasm::Exchange exchange) {
		typename Type::Module;
		{ typename Type::Module{ state }.addMemory(id, limit, exchange) } -> std::same_as<typename Type::Memory>;
		{ typename Type::Module{ state }.addTable(id, false, limit, exchange) } -> std::same_as<typename Type::Table>;
		{ typename Type::Module{ state }.addFunction(id, prototype, exchange) } -> std::same_as<typename Type::Function>;
	};

	template <wasm::IsFunction Writer> struct Function;
	template <class Writer> struct Loop;
	template <class Writer> struct Block;

	namespace detail {
		struct Pushed {
			size_t stamp = 0;
			size_t index = 0;
		};
	}

	/* any memory object to be referenced by instructions [instantiated via wasm::Module] */
	template <wasm::IsMemory Writer>
	struct Memory {
		Writer::Memory self;
		constexpr Memory(Writer::Memory&& self) : self{ std::move(self) } {}
		Memory(const wasm::Memory<Writer>&) = delete;
		constexpr Memory(wasm::Memory<Writer>&&) = default;
	};

	/* any table object to be referenced by instructions [instantiated via wasm::Module] */
	template <wasm::IsTable Writer>
	struct Table {
		Writer::Table self;
		constexpr Table(Writer::Table&& self) : self{ std::move(self) } {}
		Table(const wasm::Table<Writer>&) = delete;
		constexpr Table(wasm::Table<Writer>&&) = default;
	};

	/* any local variable to be referenced by instructions [instantiated via wasm::Function] */
	template <wasm::IsLocal Writer>
	struct Local {
		Writer::Local self;
		constexpr Local(Writer::Local&& self) : self{ std::move(self) } {}
	};

	/* any non-control-flow instruction */
	template <wasm::IsInstBase Writer>
	struct Instruction {
		Writer::InstBase self;
		constexpr Instruction(Writer::InstBase&& self) : self{ std::move(self) } {}
	};

	/* if-then block of a conditional (will automatically end the instruction-block at destruction and can be toggled to else-block) */
	template <class Writer>
	struct IfThen {
	private:
		wasm::Function<Writer>& pFunction;
		detail::Pushed pPushed;

	public:
		constexpr IfThen(wasm::Function<Writer>& fn, const wasm::Proto& prototype = {}) : pFunction{ fn } {
			pFunction.fOpen(pPushed, prototype);
		}
		IfThen(wasm::IfThen<Writer>&&) = delete;
		IfThen(const wasm::IfThen<Writer>&) = delete;
		constexpr ~IfThen() {
			pFunction.fClose(pPushed);
		}

	public:
		constexpr bool valid() const {
			return pFunction.fCheck(pPushed);
		}
		constexpr void close() {
			pFunction.fClose(pPushed);
		}
		constexpr void otherwise() {
			pFunction.fToggle(pPushed);
		}
	};

	/* target to be jumped to (inherited by wasm::Loop/wasm::Block) */
	template <wasm::IsJumpBase Writer>
	struct Target {
	protected:
		wasm::Function<Writer>& pFunction;
		detail::Pushed pPushed;
		constexpr Target(wasm::Function<Writer>& fn, const std::u8string_view& label, const wasm::Proto& prototype, bool loop) : pFunction{ fn }, self{ std::move(fn.fOpen(pPushed, label, prototype, loop)) } {}
		constexpr void fClose() {
			pFunction.fClose(pPushed);
		}

	public:
		Writer::JumpBase self;
		Target(wasm::Target<Writer>&&) = delete;
		Target(const wasm::Target<Writer>&) = delete;
		constexpr bool valid() const {
			return pFunction.fCheck(pPushed);
		}
	};

	/* loop block, which can act as wasm::Target (will automatically end the instruction-block at destruction) */
	template <class Writer>
	struct Loop : public wasm::Target<Writer> {
		constexpr Loop(wasm::Function<Writer>& fn, const std::u8string_view& label = {}, const wasm::Proto& prototype = {}) : wasm::Target<Writer>{ fn, label, prototype, true } {}
		constexpr ~Loop() {
			wasm::Target<Writer>::fClose();
		}
		constexpr bool valid() const {
			return wasm::Target<Writer>::fCheck();
		}
		constexpr void close() {
			wasm::Target<Writer>::fClose();
		}
	};

	/* jump block, which can act as wasm::Target (will automatically end the instruction-block at destruction) */
	template <class Writer>
	struct Block : public wasm::Target<Writer> {
		constexpr Block(wasm::Function<Writer>& fn, const std::u8string_view& label = {}, const wasm::Proto& prototype = {}) : wasm::Target<Writer>{ fn, label, prototype, false } {}
		constexpr ~Block() {
			wasm::Target<Writer>::fClose();
		}
		constexpr bool valid() const {
			return wasm::Target<Writer>::fCheck();
		}
		constexpr void close() {
			wasm::Target<Writer>::fClose();
		}
	};

	/* function object, which acts as instruction-sink [instantiated via wasm::Module] */
	template <wasm::IsFunction Writer>
	struct Function {
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
		Writer::Function self;

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
		constexpr void fOpen(detail::Pushed& pushed, const wasm::Proto& prototype) {
			pushed.stamp = pNext;
			pushed.index = pStack.size();
			self.pushConditional(prototype);
			pStack.emplace_back(++pNext, Opened::Type::then);
		}
		constexpr Writer::JumpBase fOpen(detail::Pushed& pushed, const std::u8string_view& label, const wasm::Proto& prototype, bool loop) {
			pushed.stamp = pNext;
			pushed.index = pStack.size();
			typename Writer::JumpBase base = self.pushJump(label, prototype, loop);
			pStack.emplace_back(++pNext, Opened::Type::target);
			return base;
		}

	public:
		constexpr Function(Writer::Function&& self) : self{ std::move(self) } {}
		Function(const wasm::Function<Writer>&) = delete;
		constexpr Function(wasm::Function<Writer>&&) = default;

	public:
		constexpr wasm::Local<Writer> local(wasm::Type type, const std::u8string_view& id = {}) {
			return wasm::Local<Writer>{ std::move(self.addLocal(type, id)) };
		}
		constexpr void operator[](const wasm::Instruction<Writer>& inst) {
			self.addInst(inst.self);
		}
	};

	/* module to instantiate memories/tables/functions [instantiated from Writer] */
	template <wasm::IsModule Writer>
	struct Module {
		Writer::Module self;
		constexpr Module(const Writer& state) : self{ state } {}
		wasm::Memory<Writer> memory(const std::u8string_view& id = {}, const wasm::Limit& limit = {}, const wasm::Exchange& exchange = {}) {
			return wasm::Memory<Writer>{ std::move(self.addMemory(id, limit, exchange)) };
		}
		wasm::Table<Writer> table(const std::u8string_view& id = {}, bool functions = true, const wasm::Limit& limit = {}, const wasm::Exchange& exchange = {}) {
			return wasm::Table<Writer>{ std::move(self.addTable(id, functions, limit, exchange)) };
		}
		wasm::Function<Writer> function(const std::u8string_view& id = {}, const wasm::Proto& prototype = {}, const wasm::Exchange& exchange = {}) {
			return wasm::Function<Writer>{ std::move(self.addFunction(id, prototype, exchange)) };
		}
	};
}
