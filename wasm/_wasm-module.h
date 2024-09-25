#pragma once

#include "wasm-common.h"

#include <vector>
#include <unordered_set>

namespace wasm {
	class _Module;
	class _Prototype;
	class _Memory;
	class _Table;

	struct _Limit {
		uint32_t min = 0;
		uint32_t max = 0;
		constexpr _Limit(uint32_t min = 0, uint32_t max = std::numeric_limits<uint32_t>::max()) : min{ min }, max{ std::max<uint32_t>(min, max) } {}
	};
	struct _Import {
		std::u8string module;
		std::u8string name;
	};
	struct _Export {
		std::u8string name;
	};

	namespace detail {
		template <class Type>
		class ModuleMember;

		struct PrototypeState {
			std::vector<wasm::Param> parameter;
			std::vector<wasm::Type> result;
			std::u8string_view id;
		};
		struct MemoryState {
			wasm::_Import imported;
			wasm::_Export exported;
			wasm::_Limit limit;
			std::u8string_view id;
		};
		struct TableState {
			wasm::_Import imported;
			wasm::_Export exported;
			wasm::_Limit limit;
			std::u8string_view id;
			bool functions = false;
		};
	}

	class _Module {
		template <class> friend class detail::ModuleMember;
	private:
		template <class Type>
		struct Types {
			std::vector<Type> list;
			std::unordered_set<std::u8string> ids;
		};

	private:
		Types<detail::PrototypeState> pPrototype;
		Types<detail::MemoryState> pMemory;
		Types<detail::TableState> pTable;

	public:
		wasm::_Prototype prototype(std::u8string_view id = {}, std::initializer_list<wasm::Param> params = {}, std::initializer_list<wasm::Type> result = {});
		wasm::_Memory memory(std::u8string_view id = {}, const wasm::_Limit& limit = {}, const wasm::_Import& imported = {}, const wasm::_Export& exported = {});
		wasm::_Table table(std::u8string_view id = {}, bool functions = true, const wasm::_Limit& limit = {}, const wasm::_Import& imported = {}, const wasm::_Export& exported = {});
	};
}
