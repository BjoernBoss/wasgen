#pragma once

#include <cinttypes>
#include <concepts>
#include <type_traits>
#include <limits>
#include <string>

namespace wasm {
	/* wasm - Limit */
	template <class Type>
	concept CanLimit = requires(Type & state, uint32_t v) {
		{ typename Type::Limit{ state, v, v } };
	};
	template <wasm::CanLimit Writer>
	struct Limit {
		typename Writer::Limit written;
		constexpr Limit(Writer& state, uint32_t min = 0, uint32_t max = std::numeric_limits<uint32_t>::max()) : written{ state, min, max } {}
	};

	/* wasm - Memory */
	template <class Type>
	concept CanMemory = wasm::CanLimit<Type> && requires(Type & state, const std::u8string_view & id, const wasm::Limit<Type>&limit) {
		{ typename Type::Memory{ state, id, limit.written } };
	};
	template <wasm::CanMemory Writer>
	struct Memory {
		typename Writer::Memory written;
		constexpr Memory(Writer& state, const std::u8string_view& id = {}, const wasm::Limit<Writer>& limit = wasm::Limit<Writer>{}) : written{ state, id, limit.written } {}
	};

	/* wasm - Table */
	template <class Type>
	concept CanTable = wasm::CanLimit<Type> && requires(Type & state, const std::u8string_view & id, const wasm::Limit<Type>&limit) {
		{ typename Type::Table{ state, id, false, limit.written } };
	};
	template <wasm::CanTable Writer>
	struct Table {
		typename Writer::Table written;
		constexpr Table(Writer& state, const std::u8string_view& id = {}, bool functions = true, const wasm::Limit<Writer>& limit = wasm::Limit<Writer>{}) : written{ state, id, functions, limit.written } {}
	};

	/* wasm - Import */
	template <class Type>
	concept CanImport = wasm::CanMemory<Type> && wasm::CanTable<Type> && requires(Type & state, const std::u8string_view & s, const wasm::Memory<Type>&memory, const wasm::Table<Type>&table) {
		{ typename Type::Import{ state, s, s, memory.written } };
		{ typename Type::Import{ state, s, s, table.written } };
	};
	template <wasm::CanImport Writer>
	struct Import {
		typename Writer::Import written;
		constexpr Import(Writer& state, const std::u8string_view& mod, const std::u8string_view& name, const wasm::Memory<Writer>& memory) : written{ state, mod, name, memory.written } {}
		constexpr Import(Writer& state, const std::u8string_view& mod, const std::u8string_view& name, const wasm::Table<Writer>& table) : written{ state, mod, name, table.written } {}
	};

	/* wasm - Export */
	template <class Type>
	concept CanExport = wasm::CanMemory<Type> && wasm::CanTable<Type> && requires(Type & state, const std::u8string_view & s, const wasm::Memory<Type>&memory, const wasm::Table<Type>&table) {
		{ typename Type::Export{ state, s, memory.written } };
		{ typename Type::Export{ state, s, table.written } };
	};
	template <wasm::CanExport Writer>
	struct Export {
		typename Writer::Export written;
		constexpr Export(Writer& state, const std::u8string_view& name, const wasm::Memory<Writer>& memory) : written{ state, name, memory.written } {}
		constexpr Export(Writer& state, const std::u8string_view& name, const wasm::Table<Writer>& table) : written{ state, name, table.written } {}
	};
}
