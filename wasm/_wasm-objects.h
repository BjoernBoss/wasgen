#pragma once

#include "wasm-common.h"
#include "_wasm-module.h"

namespace wasm {
	namespace detail {
		template <class Type>
		class ModuleMember {
		private:
			wasm::_Module& pModule;
			uint32_t pIndex{ 0 };

		protected:
			constexpr ModuleMember(wasm::_Module& module, uint32_t index) : pModule{ module }, pIndex{ index } {}

		protected:
			constexpr const Type* fGet() const {
				if constexpr (std::is_same_v<Type, detail::PrototypeState>)
					return &pModule.pPrototype.list[pIndex];
				if constexpr (std::is_same_v<Type, detail::MemoryState>)
					return &pModule.pMemory.list[pIndex];
				if constexpr (std::is_same_v<Type, detail::TableState>)
					return &pModule.pTable.list[pIndex];
				return nullptr;
			}

		public:
			constexpr const wasm::_Module& module() const {
				return pModule;
			}
			constexpr wasm::_Module& module() {
				return pModule;
			}
			constexpr std::u8string_view id() const {
				return fGet()->id;
			}
			constexpr uint32_t index() const {
				return pIndex;
			}
		};
	}

	class _Prototype : public detail::ModuleMember<detail::PrototypeState> {
		friend class wasm::_Module;
	private:
		using ModuleMember::ModuleMember;

	public:
		const std::vector<wasm::Param>& parameter() const {
			return fGet()->parameter;
		}
		const std::vector<wasm::Type>& result() const {
			return fGet()->result;
		}
	};

	class _Memory : public detail::ModuleMember<detail::MemoryState> {
		friend class wasm::_Module;
	private:
		using ModuleMember::ModuleMember;

	public:
		const wasm::_Import& imported() const {
			return fGet()->imported;
		}
		const wasm::_Export& exported() const {
			return fGet()->exported;
		}
		const wasm::_Limit& limit() const {
			return fGet()->limit;
		}
	};

	class _Table : public detail::ModuleMember<detail::TableState> {
		friend class wasm::_Module;
	private:
		using ModuleMember::ModuleMember;

	public:
		const wasm::_Import& imported() const {
			return fGet()->imported;
		}
		const wasm::_Export& exported() const {
			return fGet()->exported;
		}
		const wasm::_Limit& limit() const {
			return fGet()->limit;
		}
		bool functions() const {
			return fGet()->functions;
		}
	};
}
