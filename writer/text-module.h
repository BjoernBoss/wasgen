#pragma once

#include "text-base.h"
#include "text-sink.h"

namespace writer::text {
	using Memory = IdObject;

	using Table = IdObject;

	using Function = IdObject;

	using Global = IdObject;

	class Module {
	private:
		struct Section {
			std::unordered_set<std::u8string> ids;
			uint32_t next = 0;
		};
		struct OpenSink {
			std::u8string header;
			uint32_t parameter = uint32_t(-1);
			bool open = false;
			OpenSink() = default;
			OpenSink(std::u8string header, uint32_t param) : header{ header }, parameter{ param }, open{ true } {}
		};

	private:
		Section pTypes{};
		Section pMemory{};
		Section pTables{};
		Section pFunctions{};
		Section pGlobals{};
		std::u8string pImports;
		std::u8string pBody;
		std::vector<OpenSink> pSinks;
		std::vector<std::vector<wasm::Param>> pParameter;
		text::SinkImpl pSink;

	public:
		Module(const writer::TextWriter& state);

	public:
		text::Prototype addPrototype(const std::u8string_view& id, const wasm::Param* params, size_t paramCount, const wasm::Type* results, size_t resultCount);
		text::Memory addMemory(const std::u8string_view& id, const wasm::Limit& limit, const wasm::Exchange& exchange);
		text::Table addTable(const std::u8string_view& id, bool functions, const wasm::Limit& limit, const wasm::Exchange& exchange);
		text::Global addGlobal(const std::u8string_view& id, wasm::Type type, bool mut, const wasm::Exchange& exchange);
		text::Function addFunction(const std::u8string_view& id, const text::Prototype* prototype, const wasm::Exchange& exchange);
		text::SinkWrapper bindSink(const text::Function& fn);
		std::u8string toString();
	};
}
