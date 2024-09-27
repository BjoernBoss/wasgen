#pragma once

#include <cinttypes>
#include <vector>
#include <unordered_set>
#include <limits>
#include <string>

namespace wasm {
	class _Module;
	class _Sink;

	/* native types supported by wasm */
	enum class _Type : uint8_t {
		i32,
		i64,
		f32,
		f64,
		v128,
		refExtern,
		refFunction
	};

	/* parameter to construct any prototype-parameter */
	struct _Param {
		std::u8string id;
		wasm::_Type type;
		constexpr _Param(std::u8string id, wasm::_Type type) : id{ id }, type{ type } {}
	};

	/* limit used by memories and tables */
	struct _Limit {
		uint32_t min = 0;
		uint32_t max = 0;
		constexpr _Limit(uint32_t min = 0, uint32_t max = std::numeric_limits<uint32_t>::max()) : min{ min }, max{ std::max<uint32_t>(min, max) } {}
	};

	/* specify imports/exports for wasm-types */
	struct _Import {
		std::u8string module;
		std::u8string name;
	};
	struct _Export {
		std::u8string name;
	};

	namespace detail {
		template <class Type>
		class ModuleMember {
		private:
			wasm::_Module* pModule = 0;
			uint32_t pIndex = std::numeric_limits<uint32_t>::max();

		protected:
			constexpr ModuleMember() = default;
			constexpr ModuleMember(wasm::_Module& module, uint32_t index) : pModule{ &module }, pIndex{ index } {}

		protected:
			constexpr const Type* fGet() const;

		public:
			constexpr bool valid() const {
				return (pModule != 0);
			}
			constexpr const wasm::_Module& module() const {
				return *pModule;
			}
			constexpr wasm::_Module& module() {
				return *pModule;
			}
			constexpr std::u8string_view id() const {
				return fGet()->id;
			}
			constexpr uint32_t index() const {
				return pIndex;
			}
		};

		template <class Type>
		class SinkMember {
		protected:
			wasm::_Sink* pSink = 0;
			uint32_t pIndex = std::numeric_limits<uint32_t>::max();

		protected:
			constexpr SinkMember() = default;
			constexpr SinkMember(wasm::_Sink& sink, uint32_t index) : pSink{ &sink }, pIndex{ index } {}

		protected:
			constexpr const Type* fGet() const;

		public:
			constexpr const wasm::_Sink& sink() const {
				return *pSink;
			}
			constexpr wasm::_Sink& sink() {
				return *pSink;
			}
		};
	}
}