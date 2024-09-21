#pragma once

#include "wasm-sink.h"

namespace wasm {
	/* wrapper to define imports/exports/transports for wasm-types */
	struct Exchange {
		std::u8string expName;
		std::u8string impModule;
		std::u8string impName;
	};
	struct Transport : public wasm::Exchange {
		constexpr Transport(std::u8string expName, std::u8string impModule, std::u8string impName) : wasm::Exchange{ expName, impModule, impName } {}
	};
	struct Import : public wasm::Exchange {
		constexpr Import(std::u8string module, std::u8string name) : wasm::Exchange{ module, name, u8"" } {}
	};
	struct Export : public wasm::Exchange {
		constexpr Export(std::u8string name) : wasm::Exchange{ u8"", u8"", name } {}
	};

	/* limit used by memories and tables */
	struct Limit {
		uint32_t min = 0;
		uint32_t max = 0;
		constexpr Limit(uint32_t min = 0, uint32_t max = std::numeric_limits<uint32_t>::max()) : min{ min }, max{ std::max<uint32_t>(min, max) } {}
	};

	/* writer must provide type Module, which the wasm::Module internally instantiates
	*	must implement various interaction member-functions for it
	*	only one sink can be active at a time, and for each function only one sink can ever be created */
	template <class Type>
	concept IsModule = requires(Type state, const std::u8string_view id, const wasm::Limit limit, const wasm::Exchange exchange, const wasm::Param param, wasm::Type type, uint32_t c) {
		typename Type::Module;
		{ typename Type::Module{ state }.addPrototype(id, &param, c, &type, c) } -> std::same_as<typename Type::Prototype>;
		{ typename Type::Module{ state }.addMemory(id, limit, exchange) } -> std::same_as<typename Type::Memory>;
		{ typename Type::Module{ state }.addTable(id, false, limit, exchange) } -> std::same_as<typename Type::Table>;
		{ typename Type::Module{ state }.addGlobal(id, wasm::Type::f32, true, exchange) } -> std::same_as<typename Type::Global>;
		{ typename Type::Module{ state }.addFunction(id, std::declval<const typename Type::Prototype*>(), exchange) } -> std::same_as<typename Type::Function>;
		{ typename Type::Module{ state }.bindSink(std::declval<const typename Type::Function&>()) } -> std::same_as<typename Type::Sink>;
	};

	/* module to instantiate memories/tables/functions/globals/types [instantiated from Writer]
	*	Important: No produced/bound types must outlive the wasm::Module object
	*	Note: sinks to functions can only be created once, and binding a function implicitly unbinds the last function */
	template <wasm::IsModule Writer>
	struct Module {
		Writer::Module self;
		constexpr Module(const Writer& state) : self{ state } {}
		wasm::Prototype<Writer> prototype(const std::u8string_view& id = {}, std::initializer_list<wasm::Param> params = {}, std::initializer_list<wasm::Type> result = {}) {
			return wasm::Prototype<Writer>{ std::move(self.addPrototype(id, params.begin(), uint32_t(params.size()), result.begin(), uint32_t(result.size()))) };
		}
		wasm::Memory<Writer> memory(const std::u8string_view& id = {}, const wasm::Limit& limit = {}, const wasm::Exchange& exchange = {}) {
			return wasm::Memory<Writer>{ std::move(self.addMemory(id, limit, exchange)) };
		}
		wasm::Table<Writer> table(const std::u8string_view& id = {}, bool functions = true, const wasm::Limit& limit = {}, const wasm::Exchange& exchange = {}) {
			return wasm::Table<Writer>{ std::move(self.addTable(id, functions, limit, exchange)) };
		}
		wasm::Global<Writer> global(wasm::Type type, bool mut, const std::u8string_view& id = {}, const wasm::Exchange& exchange = {}) {
			return wasm::Global<Writer>{ std::move(self.addGlobal(id, type, mut, exchange)) };
		}
		wasm::Function<Writer> function(const std::u8string_view& id = {}, const wasm::Exchange& exchange = {}) {
			return wasm::Function<Writer>{ std::move(self.addFunction(id, 0, exchange)) };
		}
		wasm::Function<Writer> function(const wasm::Prototype<Writer>& prototype, const std::u8string_view& id = {}, const wasm::Exchange& exchange = {}) {
			return wasm::Function<Writer>{ std::move(self.addFunction(id, &prototype.self, exchange)) };
		}
		wasm::Sink<Writer> sink(const wasm::Function<Writer>& fn) {
			return wasm::Sink<Writer>{ std::move(self.bindSink(fn.self)) };
		}
	};
}
