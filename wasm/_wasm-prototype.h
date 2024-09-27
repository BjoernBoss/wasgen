#pragma once

#include "_wasm-common.h"

namespace wasm {
	namespace detail {
		struct PrototypeState {
			std::vector<wasm::_Param> parameter;
			std::vector<wasm::_Type> result;
			std::u8string_view id;
		};
	}

	class _Prototype : public detail::ModuleMember<detail::PrototypeState> {
		friend class wasm::_Module;
	public:
		constexpr _Prototype() = default;

	private:
		constexpr _Prototype(wasm::_Module& module, uint32_t index) : ModuleMember{ module, index } {}

	public:
		constexpr const std::vector<wasm::_Param>& parameter() const {
			return fGet()->parameter;
		}
		constexpr const std::vector<wasm::_Type>& result() const {
			return fGet()->result;
		}
	};
}
