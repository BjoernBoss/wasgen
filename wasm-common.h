/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2024 Bjoern Boss Henrichsen */
#pragma once

#include <ustring/ustring.h>
#include <cinttypes>
#include <vector>
#include <unordered_set>
#include <limits>
#include <string>
#include <variant>
#include <initializer_list>
#include <unordered_map>

namespace wasm {
	class Module;
	class Sink;
	class SinkInterface;
	class ModuleInterface;

	/* exception thrown when using wasm module/instructions/sinks in unsupported ways */
	struct Exception : public str::BuildException {
		template <class... Args>
		constexpr Exception(const Args&... args) : str::BuildException{ args... } {}
	};

	/* native types supported by wasm */
	enum class Type : uint8_t {
		i32,
		i64,
		f32,
		f64,
		refExtern,
		refFunction
	};

	/* exchange to define imports/exports/transports */
	struct Exchange {
	public:
		std::u8string_view importModule;
		bool exported = false;

	public:
		constexpr Exchange() = default;
		explicit constexpr Exchange(std::u8string_view importModule, bool exported) : importModule{ importModule }, exported{ exported } {}
	};

	/* explicitly mark this as an import */
	struct Import : public wasm::Exchange {
		explicit constexpr Import(std::u8string_view importModule) : wasm::Exchange{ importModule, false } {}
	};

	/* explicitly mark this as an export */
	struct Export : public wasm::Exchange {
		explicit constexpr Export() : wasm::Exchange{ u8"", true } {}
	};

	/* explicitly mark this as a transport */
	struct Transport : public wasm::Exchange {
		explicit constexpr Transport(std::u8string_view importModule) : wasm::Exchange{ importModule, true } {}
	};

	/* parameter to construct any prototype-parameter */
	struct Param {
		std::u8string id;
		wasm::Type type;
		Param(wasm::Type type) : id{}, type{ type } {}
		Param(std::u8string id, wasm::Type type) : id{ id }, type{ type } {}
	};

	/* limit used by memories and tables */
	struct Limit {
		uint32_t min = std::numeric_limits<uint32_t>::max();
		uint32_t max = std::numeric_limits<uint32_t>::max();
		constexpr Limit() = default;
		constexpr Limit(uint32_t min, uint32_t max = std::numeric_limits<uint32_t>::max()) : min{ min }, max{ std::max<uint32_t>(min, max) } {}
		constexpr bool valid() const {
			return (min != std::numeric_limits<uint32_t>::max());
		}
		constexpr bool maxValid() const {
			return (max >= min && max != std::numeric_limits<uint32_t>::max());
		}
	};

	namespace detail {
		template <class Type>
		class ModuleMember {
		private:
			mutable wasm::Module* pModule = 0;
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
			constexpr wasm::Module& module() const {
				return *pModule;
			}
			constexpr std::u8string_view id() const {
				return fGet()->id;
			}
			constexpr uint32_t index() const {
				return pIndex;
			}
			std::u8string toString() const {
				std::u8string_view id = fGet()->id;
				if (!id.empty())
					return str::Build<std::u8string>(u8"$", id);
				return str::Build<std::u8string>(pIndex);
			}
		};

		template <class Type>
		class SinkMember {
		protected:
			mutable wasm::Sink* pSink = 0;
			uint32_t pIndex = std::numeric_limits<uint32_t>::max();

		protected:
			constexpr SinkMember() = default;
			constexpr SinkMember(wasm::Sink& sink, uint32_t index) : pSink{ &sink }, pIndex{ index } {}

		protected:
			constexpr const Type* fGet() const;

		public:
			constexpr wasm::Sink& sink() const {
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
