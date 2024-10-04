#pragma once

#include "wasm-common.h"
#include "wasm-prototype.h"
#include "wasm-memory.h"
#include "wasm-table.h"
#include "wasm-global.h"
#include "wasm-function.h"
#include "wasm-value.h"

#include "../util/logging.h"

namespace wasm {
	/* module interface used to define a wasm-module */
	class ModuleInterface {
	public:
		virtual wasm::SinkInterface* sink(const wasm::Function& function) = 0;
		virtual void close(const wasm::Module& module) = 0;
		virtual void addPrototype(const wasm::Prototype& prototype) = 0;
		virtual void addMemory(const wasm::Memory& memory) = 0;
		virtual void addTable(const wasm::Table& table) = 0;
		virtual void addGlobal(const wasm::Global& global) = 0;
		virtual void addFunction(const wasm::Function& function) = 0;
		virtual void setValue(const wasm::Global& global, const wasm::Value& value) = 0;
		virtual void writeData(const wasm::Memory& memory, const wasm::Value& offset, const std::vector<uint8_t>& data) = 0;
		virtual void writeElements(const wasm::Table& table, const wasm::Value& offset, const std::vector<wasm::Value>& values) = 0;
	};

	/* write wasm-objects out to the module-implementation */
	class Module {
		template <class> friend class detail::ModuleMember;
		friend class wasm::Sink;
	private:
		template <class Type>
		struct Types {
			std::vector<Type> list;
			std::unordered_set<std::u8string> ids;
		};
		struct PrototypeList {
			wasm::Module* _this = 0;
			constexpr PrototypeList(wasm::Module* module) : _this{ module } {}
			constexpr size_t size() const {
				return _this->pPrototype.list.size();
			}
			constexpr wasm::Prototype get(uint32_t index) const {
				return wasm::Prototype{ *_this, index };
			}
		};
		struct MemoryList {
			wasm::Module* _this = 0;
			constexpr MemoryList(wasm::Module* module) : _this{ module } {}
			constexpr size_t size() const {
				return _this->pMemory.list.size();
			}
			constexpr wasm::Memory get(uint32_t index) const {
				return wasm::Memory{ *_this, index };
			}
		};
		struct TableList {
			wasm::Module* _this = 0;
			constexpr TableList(wasm::Module* module) : _this{ module } {}
			constexpr size_t size() const {
				return _this->pTable.list.size();
			}
			constexpr wasm::Table get(uint32_t index) const {
				return wasm::Table{ *_this, index };
			}
		};
		struct GlobalList {
			wasm::Module* _this = 0;
			constexpr GlobalList(wasm::Module* module) : _this{ module } {}
			constexpr size_t size() const {
				return _this->pGlobal.list.size();
			}
			constexpr wasm::Global get(uint32_t index) const {
				return wasm::Global{ *_this, index };
			}
		};
		struct FunctionList {
			wasm::Module* _this = 0;
			constexpr FunctionList(wasm::Module* module) : _this{ module } {}
			constexpr size_t size() const {
				return _this->pFunction.list.size();
			}
			constexpr wasm::Function get(uint32_t index) const {
				return wasm::Function{ *_this, index };
			}
		};

	private:
		Types<detail::PrototypeState> pPrototype;
		Types<detail::MemoryState> pMemory;
		Types<detail::TableState> pTable;
		Types<detail::GlobalState> pGlobal;
		Types<detail::FunctionState> pFunction;
		wasm::ModuleInterface* pInterface = 0;
		wasm::Prototype pNullPrototype;
		bool pImportsClosed = false;
		bool pClosed = false;

	public:
		Module(wasm::ModuleInterface* interface);
		Module() = delete;
		Module(wasm::Module&&) = delete;
		Module(const wasm::Module&) = delete;
		~Module();

	private:
		wasm::Prototype fPrototype(std::u8string_view id, std::initializer_list<wasm::Param> params, std::initializer_list<wasm::Type> result);
		wasm::Prototype fNullPrototype();
		void fCheckClosed() const;
		void fClose();

	public:
		wasm::Prototype prototype(std::u8string_view id, std::initializer_list<wasm::Param> params, std::initializer_list<wasm::Type> result);
		wasm::Memory memory(std::u8string_view id, const wasm::Limit& limit = {}, std::u8string_view importModule = {}, bool exported = false);
		wasm::Table table(std::u8string_view id, bool functions, const wasm::Limit& limit = {}, std::u8string_view importModule = {}, bool exported = false);
		wasm::Global global(std::u8string_view id, wasm::Type type, bool mutating, std::u8string_view importModule = {}, bool exported = false);
		wasm::Function function(std::u8string_view id, const wasm::Prototype& prototype = {}, std::u8string_view importModule = {}, bool exported = false);
		void value(const wasm::Global& global, const wasm::Value& value);
		void data(const wasm::Memory& memory, const wasm::Value& offset, const std::vector<uint8_t>& data);
		void elements(const wasm::Table& table, const wasm::Value& offset, const std::vector<wasm::Value>& values);
		void close();

	public:
		wasm::List<wasm::Prototype, Module::PrototypeList> prototypes() const;
		wasm::List<wasm::Memory, Module::MemoryList> memories() const;
		wasm::List<wasm::Table, Module::TableList> tables() const;
		wasm::List<wasm::Global, Module::GlobalList> globals() const;
		wasm::List<wasm::Function, Module::FunctionList> functions() const;
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
