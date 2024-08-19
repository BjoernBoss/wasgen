#pragma once

#include <cinttypes>
#include <concepts>
#include <type_traits>
#include <limits>
#include <string>
#include <vector>

namespace wasm {
	enum class Type : uint8_t {
		i32,
		i64,
		f32,
		f64,
		v128,
		refExtern,
		refFunction
	};

	/* wasm - Prototype */
	struct Parameter {
		std::u8string id;
		wasm::Type type;
		constexpr Parameter(wasm::Type type, std::u8string id = u8"") : id{ id }, type{ type } {}
	};
	struct Prototype {
		std::vector<wasm::Parameter> params;
		std::vector<wasm::Type> result;
		constexpr Prototype() = default;
		constexpr Prototype(std::initializer_list<wasm::Parameter> params, std::initializer_list<wasm::Type> result) : params{ params }, result{ result } {}
	};

	/* wasm - Local */
	struct Local {
		std::u8string id;
		wasm::Type type;
		constexpr Local(wasm::Type type, std::u8string id = u8"") : id{ id }, type{ type } {}
	};

	/* wasm - Limit */
	struct Limit {
		uint32_t min = 0;
		uint32_t max = 0;
		constexpr Limit(uint32_t min = 0, uint32_t max = std::numeric_limits<uint32_t>::max()) : min{ min }, max{ std::max<uint32_t>(min, max) } {}
	};

	/* wasm - Import/Export */
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

	/* wasm - Memory */
	template <class Type>
	concept CanMemory = requires() {
		typename Type::Memory;
	};
	template <wasm::CanMemory Writer>
	struct Memory {
		typename Writer::Memory value;
		constexpr Memory() = default;
		constexpr Memory(const typename Writer::Memory& mem) : value{ mem } {}
		constexpr Memory(typename Writer::Memory&& mem) : value{ std::move(mem) } {}
	};

	/* wasm - Table */
	template <class Type>
	concept CanTable = requires() {
		typename Type::Table;
	};
	template <wasm::CanTable Writer>
	struct Table {
		typename Writer::Table value;
		constexpr Table() = default;
		constexpr Table(const typename Writer::Table& tb) : value{ tb } {}
		constexpr Table(typename Writer::Table&& tb) : value{ std::move(tb) } {}
	};

	/* wasm - Function */
	template <class Type>
	concept CanFunction = requires(const wasm::Local local) {
		{ typename Type::Function{}.local(local) };
	};
	template <wasm::CanFunction Writer>
	struct Function {
		typename Writer::Function value;
		constexpr Function() = default;
		constexpr Function(const typename Writer::Function& fn) : value{ fn } {}
		constexpr Function(typename Writer::Function&& fn) : value{ std::move(fn) } {}
		void add(const wasm::Local& local) {
			value.local(local);
		}
	};

	/* wasm - Module */
	template <class Type>
	concept CanModule = wasm::CanMemory<Type> && wasm::CanTable<Type> && wasm::CanFunction<Type> && requires(Type state, const std::u8string_view id, const wasm::Limit limit, const wasm::Prototype prototype, const wasm::Exchange exchange) {
		{ typename Type::Module{ state }.memory(id, limit, exchange) } -> std::same_as<typename Type::Memory>;
		{ typename Type::Module{ state }.table(id, false, limit, exchange) } -> std::same_as<typename Type::Table>;
		{ typename Type::Module{ state }.function(id, prototype, exchange) } -> std::same_as<typename Type::Function>;
	};
	template <wasm::CanModule Writer>
	struct Module {
		typename Writer::Module value;
		constexpr Module(Writer& state) : value{ state } {}
		wasm::Memory<Writer> memory(const std::u8string_view& id = {}, const wasm::Limit& limit = {}, const wasm::Exchange& exchange = {}) {
			return wasm::Memory<Writer>{ value.memory(id, limit, exchange) };
		}
		wasm::Table<Writer> table(const std::u8string_view& id = {}, bool functions = true, const wasm::Limit& limit = {}, const wasm::Exchange& exchange = {}) {
			return wasm::Table<Writer>{ value.table(id, functions, limit, exchange) };
		}
		wasm::Function<Writer> function(const std::u8string_view& id = {}, const wasm::Prototype& prototype = {}, const wasm::Exchange& exchange = {}) {
			return wasm::Function<Writer>{ value.function(id, prototype, exchange) };
		}
	};
}
