#pragma once

#include <cinttypes>
#include <vector>
#include <unordered_set>
#include <limits>
#include <string>
#include <variant>
#include <initializer_list>

namespace wasm {
	class Module;
	class Sink;
	class SinkInterface;
	class ModuleInterface;

	/* native types supported by wasm */
	enum class Type : uint8_t {
		i32,
		i64,
		f32,
		f64,
		v128,
		refExtern,
		refFunction
	};

	/* parameter to construct any prototype-parameter */
	struct Param {
		std::u8string id;
		wasm::Type type;
		constexpr Param(std::u8string id, wasm::Type type) : id{ id }, type{ type } {}
	};

	/* limit used by memories and tables */
	struct Limit {
		uint32_t min = 0;
		uint32_t max = 0;
		constexpr Limit(uint32_t min = 0, uint32_t max = std::numeric_limits<uint32_t>::max()) : min{ min }, max{ std::max<uint32_t>(min, max) } {}
		constexpr bool maxValid() const {
			return (max >= min && max != std::numeric_limits<uint32_t>::max());
		}
	};

	/* specify imports/exports for wasm-types */
	struct Import {
		std::u8string module;
		std::u8string name;
		constexpr Import() = default;
		constexpr Import(std::u8string module, std::u8string name) : module{ module }, name{ name } {}
		constexpr bool valid() const {
			return (!module.empty() && !name.empty());
		}
	};
	struct Export {
		std::u8string name;
		constexpr Export() = default;
		constexpr Export(std::u8string name) : name{ name } {}
		constexpr bool valid() const {
			return !name.empty();
		}
	};

	namespace detail {
		template <class Type>
		class ModuleMember {
		private:
			wasm::Module* pModule = 0;
			uint32_t pIndex = std::numeric_limits<uint32_t>::max();

		protected:
			constexpr ModuleMember() = default;
			constexpr ModuleMember(wasm::Module& module, uint32_t index) : pModule{ &module }, pIndex{ index } {}

		protected:
			constexpr const Type* fGet() const;

		public:
			constexpr bool valid() const {
				return (pModule != 0);
			}
			constexpr const wasm::Module& module() const {
				return *pModule;
			}
			constexpr wasm::Module& module() {
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
			wasm::Sink* pSink = 0;
			uint32_t pIndex = std::numeric_limits<uint32_t>::max();

		protected:
			constexpr SinkMember() = default;
			constexpr SinkMember(wasm::Sink& sink, uint32_t index) : pSink{ &sink }, pIndex{ index } {}

		protected:
			constexpr const Type* fGet() const;

		public:
			constexpr const wasm::Sink& sink() const {
				return *pSink;
			}
			constexpr wasm::Sink& sink() {
				return *pSink;
			}
		};
	}

	/* list type used to support iterating over types created by module/sink */
	template <class Type, class ImplType>
	struct List {
	public:
		class Iterator {
			friend struct wasm::List<Type, ImplType>;
		private:
			const ImplType* pImpl = 0;
			uint32_t pIndex = 0;

		private:
			constexpr Iterator(const ImplType* impl, uint32_t index) : pImpl{ impl }, pIndex{ index } {}

		public:
			constexpr Iterator() = default;
			constexpr bool operator==(const Iterator& it) const {
				return (pImpl == it.pImpl && pIndex == it.pIndex);
			}
			constexpr bool operator!=(const Iterator& it) const {
				return !(*this == it);
			}
			constexpr Type operator*() const {
				return pImpl->get(pIndex);
			}
			constexpr Iterator& operator++() {
				++pIndex;
				return *this;
			}
			constexpr Iterator& operator--() {
				--pIndex;
				return *this;
			}
			constexpr Iterator operator++(int) {
				return Iterator{ pImpl, pIndex + 1 };
			}
			constexpr Iterator operator--(int) {
				return Iterator{ pImpl, pIndex - 1 };
			}
		};

	private:
		ImplType pImpl{};

	public:
		constexpr List(const ImplType& impl) : pImpl{ impl } {}
		constexpr Type operator[](size_t index) const {
			return pImpl.get(uint32_t(index));
		}
		constexpr size_t size() const {
			return pImpl.size();
		}
		constexpr bool empty() const {
			return (pImpl.size() == 0);
		}
		constexpr Iterator begin() const {
			return Iterator{ &pImpl, 0 };
		}
		constexpr Iterator end() const {
			return Iterator{ &pImpl, uint32_t(pImpl.size()) };
		}
	};
}
