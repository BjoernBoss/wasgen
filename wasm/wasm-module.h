#pragma once

#include "wasm-sink.h"

namespace wasm {
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

	/* writer must provide type Module, which the wasm::Module internally instantiates
	*	must implement various interaction member-functions for it */
	template <class Type>
	concept IsModule = requires(Type state, const std::u8string_view id, const wasm::Limit limit, const wasm::Proto prototype, const wasm::Exchange exchange) {
		typename Type::Module;
		{ typename Type::Module{ state }.addMemory(id, limit, exchange) } -> std::same_as<typename Type::Memory>;
		{ typename Type::Module{ state }.addTable(id, false, limit, exchange) } -> std::same_as<typename Type::Table>;
		{ typename Type::Module{ state }.addGlobal(id, wasm::Type::f32, exchange) } -> std::same_as<typename Type::Global>;
		{ typename Type::Module{ state }.addFunction(id, prototype, exchange) } -> std::same_as<typename Type::Function>;
		{ typename Type::Module{ state }.bindSink(std::declval<const typename Type::Function>()) } -> std::same_as<typename Type::Sink>;
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
		wasm::Global<Writer> global(wasm::Type type, const std::u8string_view& id = {}, const wasm::Exchange& exchange = {}) {
			return wasm::Global<Writer>{ std::move(self.addGlobal(id, type, exchange)) };
		}
		wasm::Function<Writer> function(const std::u8string_view& id = {}, const wasm::Proto& prototype = {}, const wasm::Exchange& exchange = {}) {
			return wasm::Function<Writer>{ std::move(self.addFunction(id, prototype, exchange)) };
		}
		wasm::Sink<Writer> bind(const wasm::Function<Writer>& fn) {
			return wasm::Sink<Writer>{ std::move(self.bindSink(fn.self)) };
		}
	};
}
