#pragma once

#include "_wasm-common.h"
#include "_wasm-prototype.h"
#include "_wasm-memory.h"
#include "_wasm-table.h"
#include "_wasm-global.h"
#include "_wasm-function.h"

namespace wasm {
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
		Types<detail::GlobalState> pGlobal;
		Types<detail::FunctionState> pFunction;

	public:
		wasm::_Prototype prototype(std::initializer_list<wasm::_Param> params, std::initializer_list<wasm::_Type> result, std::u8string_view id = {});
		wasm::_Memory memory(const wasm::_Limit& limit = {}, std::u8string_view id = {}, const wasm::_Import& imported = {}, const wasm::_Export& exported = {});
		wasm::_Table table(bool functions, const wasm::_Limit& limit = {}, std::u8string_view id = {}, const wasm::_Import& imported = {}, const wasm::_Export& exported = {});
		wasm::_Global global(wasm::_Type type, bool mutating, std::u8string_view id = {}, const wasm::_Import& imported = {}, const wasm::_Export& exported = {});
		wasm::_Function function(const wasm::_Prototype& prototype = {}, std::u8string_view id = {}, const wasm::_Import& imported = {}, const wasm::_Export& exported = {});
	};

	namespace detail {
		template <class Type>
		constexpr const Type* ModuleMember<Type>::fGet() const {
			if constexpr (std::is_same_v<Type, detail::PrototypeState>)
				return &pModule->pPrototype.list[pIndex];
			if constexpr (std::is_same_v<Type, detail::MemoryState>)
				return &pModule->pMemory.list[pIndex];
			if constexpr (std::is_same_v<Type, detail::TableState>)
				return &pModule->pTable.list[pIndex];
			if constexpr (std::is_same_v<Type, detail::GlobalState>)
				return &pModule->pGlobal.list[pIndex];
			if constexpr (std::is_same_v<Type, detail::FunctionState>)
				return &pModule->pFunction.list[pIndex];
			return nullptr;
		}
	}
}
