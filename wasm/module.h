#pragma once

#include <string>
#include <vector>
#include <limits>
#include <initializer_list>
#include <utility>

namespace wasm {
	enum class Type : uint8_t {
		i32,
		i64,
		f32,
		f64,
		v128,
		refExtern,
		refFunction
	};
	struct Import {
		std::u8string_view module;
		std::u8string_view name;
	};
	struct Limits {
	public:
		size_t min;
		size_t max;

	public:
		Limits(size_t mn = 0, size_t mx = std::numeric_limits<size_t>::max()) : min{ mn }, max{ mx } {}
	};

	struct ParamType {
		std::u8string_view name;
		wasm::Type type;
	};
	struct FunctionType {
		std::vector<wasm::ParamType> params;
		std::vector<wasm::Type> result;
	};
	struct MemoryType {
		wasm::Limits limits;
	};
	struct TableType {
		wasm::Limits limits;
		bool functions = false;
	};

	struct InstructionSink {
	};

	struct SExpression {

	};

	class Module {
	public:
		size_t importFunction(const wasm::Import& name, const std::u8string_view& id, const wasm::FunctionType& type) {
			return 0;
		}
		size_t importMemory(const wasm::Import& name, const std::u8string_view& id, const wasm::MemoryType& type) {
			return 0;
		}
		size_t importTable(const wasm::Import& name, const std::u8string_view& id, const wasm::TableType& type) {
			return 0;
		}
		std::pair<size_t, wasm::InstructionSink> addFunction(const std::u8string_view& id, const wasm::FunctionType& type, const std::u8string_view& exportName = {}) {
			return { 0, {} };
		}
		size_t addMemory(const std::u8string_view& id, const wasm::MemoryType& type, const std::u8string_view& exportName = {}) {
			return 0;
		}
		size_t addTable(const std::u8string_view& id, const wasm::TableType& type, const std::u8string_view& exportName = {}) {
			return 0;
		}
	};

	namespace fn {
		static wasm::FunctionType VoidFunction = wasm::FunctionType{ { }, { } };
		static wasm::FunctionType i32Function = { { }, { wasm::Type::i32 } };
		static wasm::FunctionType i64Function = { { }, { wasm::Type::i64 } };
	}
}
