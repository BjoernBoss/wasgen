#pragma once

#include "_wasm-common.h"
#include "_wasm-prototype.h"
#include "_wasm-memory.h"
#include "_wasm-table.h"
#include "_wasm-global.h"
#include "_wasm-function.h"

namespace wasm {
	/* module interface used to define a wasm-module */
	class _ModuleInterface {
	public:
		virtual wasm::_SinkInterface* sink(const wasm::_Function& function) = 0;
		virtual void close(const wasm::_Module& module) = 0;
		virtual void addPrototype(const wasm::_Prototype& prototype) = 0;
		virtual void addMemory(const wasm::_Memory& memory) = 0;
		virtual void addTable(const wasm::_Table& table) = 0;
		virtual void addGlobal(const wasm::_Global& global) = 0;
		virtual void addFunction(const wasm::_Function& function) = 0;
	};

	/* write wasm-objects out to the module-implementation */
	class _Module {
		template <class> friend class detail::ModuleMember;
		friend class wasm::_Sink;
	private:
		template <class Type>
		struct Types {
			std::vector<Type> list;
			std::unordered_set<std::u8string> ids;
		};
		struct _PrototypeList {
			wasm::_Module* _this = 0;
			constexpr _PrototypeList(wasm::_Module* module) : _this{ module } {}
			constexpr size_t size() const {
				return _this->pPrototype.list.size();
			}
			constexpr wasm::_Prototype get(uint32_t index) const {
				return wasm::_Prototype{ *_this, index };
			}
		};
		struct _MemoryList {
			wasm::_Module* _this = 0;
			constexpr _MemoryList(wasm::_Module* module) : _this{ module } {}
			constexpr size_t size() const {
				return _this->pMemory.list.size();
			}
			constexpr wasm::_Memory get(uint32_t index) const {
				return wasm::_Memory{ *_this, index };
			}
		};
		struct _TableList {
			wasm::_Module* _this = 0;
			constexpr _TableList(wasm::_Module* module) : _this{ module } {}
			constexpr size_t size() const {
				return _this->pTable.list.size();
			}
			constexpr wasm::_Table get(uint32_t index) const {
				return wasm::_Table{ *_this, index };
			}
		};
		struct _GlobalList {
			wasm::_Module* _this = 0;
			constexpr _GlobalList(wasm::_Module* module) : _this{ module } {}
			constexpr size_t size() const {
				return _this->pGlobal.list.size();
			}
			constexpr wasm::_Global get(uint32_t index) const {
				return wasm::_Global{ *_this, index };
			}
		};
		struct _FunctionList {
			wasm::_Module* _this = 0;
			constexpr _FunctionList(wasm::_Module* module) : _this{ module } {}
			constexpr size_t size() const {
				return _this->pFunction.list.size();
			}
			constexpr wasm::_Function get(uint32_t index) const {
				return wasm::_Function{ *_this, index };
			}
		};

	private:
		Types<detail::PrototypeState> pPrototype;
		Types<detail::MemoryState> pMemory;
		Types<detail::TableState> pTable;
		Types<detail::GlobalState> pGlobal;
		Types<detail::FunctionState> pFunction;
		wasm::_ModuleInterface* pInterface = 0;
		bool pClosed = false;

	public:
		_Module(wasm::_ModuleInterface* interface);
		_Module() = delete;
		_Module(wasm::_Module&&) = delete;
		_Module(const wasm::_Module&) = delete;
		~_Module();

	private:
		void fClose();
		void fCheckClosed() const;

	public:
		wasm::_Prototype prototype(std::initializer_list<wasm::_Param> params, std::initializer_list<wasm::_Type> result, std::u8string_view id = {});
		wasm::_Memory memory(const wasm::_Limit& limit = {}, std::u8string_view id = {}, const wasm::_Import& imported = {}, const wasm::_Export& exported = {});
		wasm::_Table table(bool functions, const wasm::_Limit& limit = {}, std::u8string_view id = {}, const wasm::_Import& imported = {}, const wasm::_Export& exported = {});
		wasm::_Global global(wasm::_Type type, bool mutating, std::u8string_view id = {}, const wasm::_Import& imported = {}, const wasm::_Export& exported = {});
		wasm::_Function function(const wasm::_Prototype& prototype = {}, std::u8string_view id = {}, const wasm::_Import& imported = {}, const wasm::_Export& exported = {});
		void close();

	public:
		wasm::_List<wasm::_Prototype, _Module::_PrototypeList> prototypes() const;
		wasm::_List<wasm::_Memory, _Module::_MemoryList> memories() const;
		wasm::_List<wasm::_Table, _Module::_TableList> tables() const;
		wasm::_List<wasm::_Global, _Module::_GlobalList> globals() const;
		wasm::_List<wasm::_Function, _Module::_FunctionList> functions() const;
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
