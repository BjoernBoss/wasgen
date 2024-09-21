#pragma once

#include <ustring/ustring.h>
#include <algorithm>
#include <unordered_set>

#include "../wasm/wasm.h"
#include "../util/logging.h"

struct TextWriter {
private:
	static std::u8string_view fMakeType(wasm::Type type) {
		switch (type) {
		case wasm::Type::i64:
			return u8"i64";
		case wasm::Type::f32:
			return u8"f32";
		case wasm::Type::f64:
			return u8"f64";
		case wasm::Type::refExtern:
			return u8"externref";
		case wasm::Type::refFunction:
			return u8"funcref";
		case wasm::Type::v128:
			return u8"v128";
		case wasm::Type::i32:
		default:
			return u8"i32";
		}
	}
	static std::u8string fMakeId(const std::u8string_view& id) {
		if (id.empty())
			return std::u8string();
		return str::Build<std::u8string>(U" $", id);
	}
	static std::u8string fMakeLimit(const wasm::Limit& limit) {
		if (limit.max == std::numeric_limits<uint32_t>::max())
			return str::Build<std::u8string>(u8' ', limit.min);
		return str::Build<std::u8string>(u8' ', limit.min, u8' ', limit.max);
	}
	static std::u8string fMakeExchange(const wasm::Exchange& exchange) {
		std::u8string out;
		if (!exchange.expName.empty())
			str::FormatTo(out, u8" (export \"{}\")", exchange.expName);
		if (!exchange.impName.empty())
			str::FormatTo(out, u8" (import \"{}\" \"{}\")", exchange.impModule, exchange.impName);
		return out;
	}

private:
	struct Section {
		std::unordered_set<std::u8string> ids;
		uint32_t next = 0;
	};
	struct IdObject {
		std::u8string id;
		uint32_t index = 0;
		constexpr IdObject(std::u8string id, uint32_t index) : id{ id }, index{ index } {}
		constexpr std::u8string toString() const {
			if (id.empty())
				return str::Int<std::u8string>(index);
			return id;
		}
	};
	class SinkBase {
	private:
		std::u8string pOutput;
		std::u8string pDepth;

	public:
		constexpr SinkBase(std::u8string output) : pOutput{ output }, pDepth{ u8"\n  " } {}

	public:
		constexpr void push(const std::u8string_view& name) {
			str::BuildTo(pOutput, pDepth, u8'(', name);
			pDepth.append(u8"  ");
		}
		constexpr void pop() {
			pDepth.resize(pDepth.size() - 2);
			str::BuildTo(pOutput, pDepth, u8')');
		}
		constexpr void addLine(const std::u8string_view& str) {
			str::BuildTo(pOutput, pDepth, str);
		}
		constexpr void finalize(std::u8string& out) const {
			if (!pOutput.empty())
				str::BuildTo(out, pOutput, u8')');
		}
	};

public:
	/* wasm::IsVariable */
	struct Variable {
		std::u8string id;
		constexpr Variable(std::u8string id) : id{ id } {}
	};

	/* wasm::IsInstruction */
	struct Instruction {
		std::u8string value;
		constexpr Instruction(std::u8string value) : value{ value } {}
	};

	/* wasm::IsTarget */
	struct Target {
		std::u8string id;
		constexpr Target(std::u8string id) : id{ id } {}
	};





	/* wasm::IsPrototype */
	using Prototype = IdObject;

	/* wasm::IsMemory */
	using Memory = IdObject;

	/* wasm::IsTable */
	using Table = IdObject;

	/* wasm::IsFunction */
	using Function = IdObject;

	/* wasm::IsGlobal */
	using Global = IdObject;

	/* wasm::IsSink */
	class Sink {
	private:
		std::u8string pOutput;
		std::u8string pDepth;

	public:
		constexpr Sink(std::u8string output) : pOutput{ output }, pDepth{ u8"\n  " } {}

	private:
		constexpr void fPush() {
			pDepth.append(u8"  ");
		}
		constexpr void fPop() {
			pDepth.resize(pDepth.size() - 2);
		}
		constexpr void fAddLine(const std::u8string_view& str) {
			pOutput.append(1, u8'\n').append(pDepth).append(str);
		}

	public:
		constexpr void finalize(std::u8string& out) const {
			if (!pOutput.empty())
				str::BuildTo(out, pOutput, u8')');
		}

	public:
		constexpr TextWriter::Target pushLoop(const std::u8string_view& label, const TextWriter::Prototype* prototype) {
			return TextWriter::Target{ std::u8string(label) };
		}
		constexpr void popLoop(TextWriter::Target& target) {
		}

		constexpr TextWriter::Target pushBlock(const std::u8string_view& label, const TextWriter::Prototype* prototype) {
			return TextWriter::Target{ std::u8string(label) };
		}
		constexpr void popBlock(TextWriter::Target& target) {
		}
		constexpr TextWriter::Target pushConditional(const std::u8string_view& label, const TextWriter::Prototype* prototype) {
			return TextWriter::Target{ std::u8string(label) };
		}
		constexpr void toggleConditional(TextWriter::Target& target) {
		}
		constexpr void popConditional(TextWriter::Target& target) {
		}

		constexpr TextWriter::Variable getParameter(uint32_t index) {
			return TextWriter::Variable{ std::u8string() };
		}
		constexpr TextWriter::Variable addLocal(wasm::Type type, std::u8string_view id) {
			return TextWriter::Variable{ std::u8string(id) };
		}
		constexpr void addInst(const TextWriter::Instruction& inst) {

		}
		constexpr void close() {

		}
	};

	/* wasm::IsModule */
	class Module {
	private:
		Section pTypes{};
		Section pMemory{};
		Section pTables{};
		Section pFunctions{};
		Section pGlobals{};
		std::u8string pImports;
		std::u8string pBody;
		std::vector<std::u8string> pSinks;


		std::u8string out;

	public:
		Module(const TextWriter& state) {}

	public:
		constexpr TextWriter::Prototype addPrototype(const std::u8string_view& id, const wasm::Param* params, size_t paramCount, const wasm::Type* results, size_t resultCount) {
			/* validate the id */
			std::u8string _id{ id };
			if (!_id.empty() && pTypes.ids.count(_id) != 0)
				util::fail(u8"Prototype with id [", _id, "] already defined");

			/* allocate the next id-object */
			if (!_id.empty())
				pTypes.ids.insert(_id);
			TextWriter::IdObject obj{ std::move(_id), pTypes.next++ };

			/* write the actual type to the output */
			str::BuildTo(pBody, u8"\n (type", TextWriter::fMakeId(id), u8" (func");
			for (size_t i = 0; i < paramCount; ++i)
				str::BuildTo(pBody, u8" (param", TextWriter::fMakeId(id), u8' ', TextWriter::fMakeType(params[i].type), u8')');
			if (resultCount > 0) {
				pBody.append(u8" (result ");
				for (size_t i = 0; i < resultCount; ++i)
					str::BuildTo(pBody, TextWriter::fMakeType(results[i]), u8' ');
				pBody.push_back(u8')');
			}
			pBody.append(u8"))");

			return obj;
		}
		constexpr TextWriter::Memory addMemory(const std::u8string_view& id, const wasm::Limit& limit, const wasm::Exchange& exchange) {
			/* validate the id */
			std::u8string _id{ id };
			if (!_id.empty() && pMemory.ids.count(_id) != 0)
				util::fail(u8"Memory with id [", _id, "] already defined");

			/* allocate the next id-object */
			if (!_id.empty())
				pMemory.ids.insert(_id);
			TextWriter::IdObject obj{ std::move(_id), pMemory.next++ };

			/* write the actual memory to the output */
			str::BuildTo((exchange.impName.empty() ? pBody : pImports), u8"\n  (memory", fMakeId(id), fMakeExchange(exchange), fMakeLimit(limit), u8')');

			return obj;
		}
		constexpr TextWriter::Table addTable(const std::u8string_view& id, bool functions, const wasm::Limit& limit, const wasm::Exchange& exchange) {
			/* validate the id */
			std::u8string _id{ id };
			if (!_id.empty() && pTables.ids.count(_id) != 0)
				util::fail(u8"Table with id [", _id, "] already defined");

			/* allocate the next id-object */
			if (!_id.empty())
				pTables.ids.insert(_id);
			TextWriter::IdObject obj{ std::move(_id), pTables.next++ };

			/* write the actual table to the output */
			str::BuildTo((exchange.impName.empty() ? pBody : pImports), u8"\n  (table", fMakeId(id), fMakeExchange(exchange), fMakeLimit(limit), functions ? u8" funcref)" : u8" externref)");

			return obj;
		}
		constexpr TextWriter::Global addGlobal(const std::u8string_view& id, wasm::Type type, bool mut, const wasm::Exchange& exchange) {
			/* validate the id */
			std::u8string _id{ id };
			if (!_id.empty() && pGlobals.ids.count(_id) != 0)
				util::fail(u8"Globals with id [", _id, "] already defined");

			/* allocate the next id-object */
			if (!_id.empty())
				pGlobals.ids.insert(_id);
			TextWriter::IdObject obj{ std::move(_id), pTables.next++ };

			/* write the actual global to the output */
			std::u8string typeString = (mut ? str::Build<std::u8string>(u8"(mut ", fMakeType(type), u8')') : std::u8string(fMakeType(type)));
			str::BuildTo((exchange.impName.empty() ? pBody : pImports), u8"\n  (global", fMakeId(id), fMakeExchange(exchange), u8' ', typeString, u8')');

			return obj;
		}
		constexpr TextWriter::Function addFunction(const std::u8string_view& id, const TextWriter::Prototype* prototype, const wasm::Exchange& exchange) {
			/* validate the id */
			std::u8string _id{ id };
			if (!_id.empty() && pFunctions.ids.count(_id) != 0)
				util::fail(u8"Function with id [", _id, "] already defined");

			/* allocate the next id-object */
			if (!_id.empty())
				pFunctions.ids.insert(_id);
			TextWriter::IdObject obj{ std::move(_id), pFunctions.next++ };

			/* construct the initial function description */
			std::u8string funcText;
			str::BuildTo(funcText, u8"\n  (func", fMakeId(id), fMakeExchange(exchange));
			if (prototype != 0)
				str::BuildTo(funcText, u8" (type ", prototype->toString(), u8')');

			/* check if this is not an import, in which case a proper sink needs to be set-up, otherwise it can be produced immediately */
			if (exchange.impName.empty())
				pSinks.emplace_back(std::move(funcText));
			else {
				pSinks.emplace_back();
				pImports.append(funcText);
			}

			return obj;
		}
		constexpr TextWriter::Sink bindSink(const TextWriter::Function& fn) {
			/* check if the sink is still available */
			if (pSinks[fn.index].empty())
				util::fail(u8"Cannot construct a sink to function [", fn.toString(), "] as it is marked as an import");
			std::u8string content;
			std::swap(content, pSinks[fn.index]);

			/* close the last sink */
			return TextWriter::Sink{ pSinks[fn.index] };
		}
		constexpr std::u8string toString() const {
			if (out.empty())
				return u8"(module)";
			return str::Build<std::u8string>(u8"(module", out, u8"\n)");
		}
	};

	/* wasm::IsInst */
	struct Inst {
		static constexpr TextWriter::Instruction Consti32(uint32_t v) {
			return TextWriter::Instruction{ str::Build<std::u8string>(u8"i32.const ", v) };
		}
		static constexpr TextWriter::Instruction Consti64(uint64_t v) {
			return TextWriter::Instruction{ str::Build<std::u8string>(u8"i64.const ", v) };
		}
		static constexpr TextWriter::Instruction Constf32(float v) {
			return TextWriter::Instruction{ str::Build<std::u8string>(u8"f32.const ", v) };
		}
		static constexpr TextWriter::Instruction Constf64(double v) {
			return TextWriter::Instruction{ str::Build<std::u8string>(u8"f64.const ", v) };
		}
		static constexpr TextWriter::Instruction NoOp(wasm::NoOpType op, wasm::OperandType type) {
			return TextWriter::Instruction{ u8"%not-implemented%" };
		}
		static constexpr TextWriter::Instruction Memory(wasm::MemOpType op, const TextWriter::Memory* mem, const TextWriter::Memory* src, uint32_t off, wasm::OperandType type) {
			return TextWriter::Instruction{ u8"%not-implemented%" };
		}
		static constexpr TextWriter::Instruction Table(wasm::TabOpType op, const TextWriter::Table* tab, const TextWriter::Table* src) {
			return TextWriter::Instruction{ u8"%not-implemented%" };
		}
		static constexpr TextWriter::Instruction Local(wasm::LocOpType op, const TextWriter::Variable& loc) {
			return TextWriter::Instruction{ u8"%not-implemented%" };
		}
		static constexpr TextWriter::Instruction Global(wasm::GlobOpType op, const TextWriter::Global& glob) {
			return TextWriter::Instruction{ u8"%not-implemented%" };
		}
		static constexpr TextWriter::Instruction Ref(wasm::RefOpType op, const TextWriter::Function* fn) {
			return TextWriter::Instruction{ u8"%not-implemented%" };
		}
		static constexpr TextWriter::Instruction Call(wasm::CallOpType op, const TextWriter::Function& fn) {
			return TextWriter::Instruction{ u8"%not-implemented%" };
		}
		static constexpr TextWriter::Instruction Indirect(wasm::CallOpType op, const TextWriter::Table* tab, const TextWriter::Prototype* type) {
			return TextWriter::Instruction{ u8"%not-implemented%" };
		}
		static constexpr TextWriter::Instruction Branch(wasm::BrOpType op, const TextWriter::Target& tgt, const TextWriter::Target* list, uint32_t count) {
			return TextWriter::Instruction{ u8"%not-implemented%" };
		}
	};
};
