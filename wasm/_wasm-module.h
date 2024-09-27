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
		wasm::_Prototype prototype(std::u8string_view id = {}, std::initializer_list<wasm::_Param> params = {}, std::initializer_list<wasm::_Type> result = {});
		wasm::_Memory memory(std::u8string_view id = {}, const wasm::_Limit& limit = {}, const wasm::_Import& imported = {}, const wasm::_Export& exported = {});
		wasm::_Table table(std::u8string_view id = {}, bool functions = true, const wasm::_Limit& limit = {}, const wasm::_Import& imported = {}, const wasm::_Export& exported = {});
		wasm::_Global global(wasm::_Type type, bool mutating, std::u8string_view id = {}, const wasm::_Import& imported = {}, const wasm::_Export& exported = {});
		wasm::_Function function(std::u8string_view id = {}, const wasm::_Prototype& prototype = {}, const wasm::_Import& imported = {}, const wasm::_Export& exported = {});
	};
}
