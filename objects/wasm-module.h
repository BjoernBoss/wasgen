/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2024 Bjoern Boss Henrichsen */
#pragma once

#include "../wasm-common.h"
#include "wasm-prototype.h"
#include "wasm-memory.h"
#include "wasm-table.h"
#include "wasm-global.h"
#include "wasm-function.h"
#include "wasm-value.h"

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
		virtual void writeData(const wasm::Memory& memory, const wasm::Value& offset, const uint8_t* data, uint32_t count) = 0;
		virtual void writeElements(const wasm::Table& table, const wasm::Value& offset, const wasm::Value* values, uint32_t count) = 0;
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
		struct PrototypeKey {
			std::vector<wasm::Type> list;
			size_t params = 0;
		};
		struct PrototypeKeyOps {
			std::size_t operator()(const Module::PrototypeKey& k) const {
				std::size_t h0 = std::hash<size_t>{}(k.params);
				std::size_t h1 = std::hash<std::u8string_view>{}(std::u8string_view{ reinterpret_cast<const char8_t*>(k.list.data()), sizeof(wasm::Type) * k.list.size() });
				return h0 ^ (h1 << 1);
			}
			bool operator()(const Module::PrototypeKey& l, const Module::PrototypeKey& r) const {
				return (l.params == r.params && l.list == r.list);
			}
		};

	private:
		std::unordered_map<Module::PrototypeKey, uint32_t, PrototypeKeyOps, PrototypeKeyOps> pAnonTypes;
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
		~Module() noexcept(false);

	private:
		wasm::Prototype fPrototype(std::u8string_view id, std::initializer_list<wasm::Param> params, std::initializer_list<wasm::Type> result);
		wasm::Prototype fPrototype(std::initializer_list<wasm::Type> params, std::initializer_list<wasm::Type> result);
		wasm::Function fFunction(std::u8string_view id, const wasm::Prototype& prototype, const wasm::Exchange& exchange);
		void fData(const wasm::Memory& memory, const wasm::Value& offset, const uint8_t* data, uint32_t count);
		void fElements(const wasm::Table& table, const wasm::Value& offset, const wasm::Value* values, uint32_t count);
		void fCheckClosed() const;
		void fClose();

	public:
		wasm::Prototype prototype(std::initializer_list<wasm::Type> params, std::initializer_list<wasm::Type> result);
		wasm::Prototype prototype(std::u8string_view id, std::initializer_list<wasm::Param> params, std::initializer_list<wasm::Type> result);
		wasm::Memory memory(std::u8string_view id, const wasm::Limit& limit, const wasm::Exchange& exchange = {});
		wasm::Table table(std::u8string_view id, bool functions, const wasm::Limit& limit, const wasm::Exchange& exchange = {});
		wasm::Global global(std::u8string_view id, wasm::Type type, bool mutating, const wasm::Exchange& exchange = {});
		wasm::Function function(std::u8string_view id, const wasm::Prototype& prototype, const wasm::Exchange& exchange = {});
		wasm::Function function(std::u8string_view id, std::initializer_list<wasm::Type> params, std::initializer_list<wasm::Type> result, const wasm::Exchange& exchange = {});
		void value(const wasm::Global& global, const wasm::Value& value);
		void data(const wasm::Memory& memory, const wasm::Value& offset, const std::vector<uint8_t>& data);
		void data(const wasm::Memory& memory, const wasm::Value& offset, const uint8_t* data, size_t count);
		void elements(const wasm::Table& table, const wasm::Value& offset, const std::vector<wasm::Value>& values);
		void elements(const wasm::Table& table, const wasm::Value& offset, const wasm::Value* values, size_t count);
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

	constexpr const std::vector<wasm::Param>& wasm::Prototype::parameter() const {
		return fGet()->parameter;
	}
	constexpr const std::vector<wasm::Type>& wasm::Prototype::result() const {
		return fGet()->result;
	}
	constexpr bool wasm::Memory::imported() const {
		return !fGet()->importModule.empty();
	}
	constexpr bool wasm::Memory::exported() const {
		return fGet()->exported;
	}
	constexpr const std::u8string& wasm::Memory::importModule() const {
		return fGet()->importModule;
	}
	constexpr const wasm::Limit& wasm::Memory::limit() const {
		return fGet()->limit;
	}
	constexpr bool wasm::Table::imported() const {
		return !fGet()->importModule.empty();
	}
	constexpr bool wasm::Table::exported() const {
		return fGet()->exported;
	}
	constexpr const std::u8string& wasm::Table::importModule() const {
		return fGet()->importModule;
	}
	constexpr const wasm::Limit& wasm::Table::limit() const {
		return fGet()->limit;
	}
	constexpr bool wasm::Table::functions() const {
		return fGet()->functions;
	}
	constexpr bool wasm::Global::imported() const {
		return !fGet()->importModule.empty();
	}
	constexpr bool wasm::Global::exported() const {
		return fGet()->exported;
	}
	constexpr const std::u8string& wasm::Global::importModule() const {
		return fGet()->importModule;
	}
	constexpr wasm::Type wasm::Global::type() const {
		return fGet()->type;
	}
	constexpr bool wasm::Global::mutating() const {
		return fGet()->mutating;
	}
	constexpr bool wasm::Function::imported() const {
		return !fGet()->importModule.empty();
	}
	constexpr bool wasm::Function::exported() const {
		return fGet()->exported;
	}
	constexpr const std::u8string& wasm::Function::importModule() const {
		return fGet()->importModule;
	}
	constexpr wasm::Prototype wasm::Function::prototype() const {
		return fGet()->prototype;
	}
}
