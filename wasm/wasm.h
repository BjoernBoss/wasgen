#pragma once

#include <cinttypes>
#include <concepts>
#include <type_traits>
#include <limits>
#include <string>

namespace wasm {
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
	concept CanMemory = requires(Type & state, const std::u8string_view & id, const wasm::Limit & limit, const wasm::Exchange & exchange) {
		{ typename Type::Memory{ state, id, limit, exchange } };
	};
	template <wasm::CanMemory Writer>
	struct Memory {
		typename Writer::Memory value;
		constexpr Memory(Writer& state, const std::u8string_view& id = {}, const wasm::Limit& limit = {}, const wasm::Exchange& exchange = {}) : value{ state, id, limit, exchange } {}
	};

	/* wasm - Table */
	template <class Type>
	concept CanTable = requires(Type & state, const std::u8string_view & id, const wasm::Limit & limit, const wasm::Exchange & exchange) {
		{ typename Type::Table{ state, id, false, limit, exchange } };
	};
	template <wasm::CanTable Writer>
	struct Table {
		typename Writer::Table value;
		constexpr Table(Writer& state, const std::u8string_view& id = {}, bool functions = true, const wasm::Limit& limit = {}, const wasm::Exchange& exchange = {}) : value{ state, id, functions, limit, exchange } {}
	};

	/* wasm - Module */
	template <class Type>
	concept CanModule = wasm::CanMemory<Type> && wasm::CanTable<Type> && requires(Type & state, const wasm::Memory<Type>&memory, const wasm::Table<Type>&table) {
		{ typename Type::Module{ state }.addMemory(memory.value) };
		{ typename Type::Module{ state }.addTable(table.value) };
	};
	template <wasm::CanModule Writer>
	struct Module {
		typename Writer::Module value;
		constexpr Module(Writer& state) : value{ state } {}
		void add(const wasm::Memory<Writer>& memory) {
			value.addMemory(memory.value);
		}
		void add(const wasm::Table<Writer>& table) {
			value.addTable(table.value);
		}
	};
}
