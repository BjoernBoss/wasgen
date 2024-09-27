#pragma once

#include "wasm-common.h"

namespace wasm {
	namespace detail {
		struct PrototypeState {
			std::vector<wasm::Param> parameter;
			std::vector<wasm::Type> result;
			std::u8string_view id;
		};
	}

	/* describe a wasm-prototype object */
	class Prototype : public detail::ModuleMember<detail::PrototypeState> {
		friend class wasm::Module;
	public:
		constexpr Prototype() = default;

	private:
		constexpr Prototype(wasm::Module& module, uint32_t index) : ModuleMember{ module, index } {}

	public:
		constexpr const std::vector<wasm::Param>& parameter() const {
			return fGet()->parameter;
		}
		constexpr const std::vector<wasm::Type>& result() const {
			return fGet()->result;
		}
	};
}
