#pragma once

#include "text-base.h"

namespace writer::text {
	class SinkImpl;

	struct Target {
		text::SinkImpl& sink;
		size_t stamp = 0;
		size_t index = 0;
		Target(text::SinkImpl& sink, size_t stamp, size_t index);
		std::u8string toString() const;
	};

	using Variable = text::IdObject;

	class SinkImpl {
	private:
		struct Pushed {
			std::unordered_set<std::u8string>::iterator label;
			size_t stamp = 0;
			enum class Type : uint8_t {
				target,
				then,
				otherwise
			} type = Type::target;
		};

	private:
		std::vector<wasm::Param> pParameter;
		std::unordered_set<std::u8string> pLocals;
		std::unordered_set<std::u8string> pLabels;
		std::vector<Pushed> pPushed;
		std::u8string pOutput;
		std::u8string pBody;
		std::u8string pDepth;
		size_t pStamp = 0;
		size_t pInstance = 0;
		uint32_t pLocalIndex = 0;
		bool pClosed = false;
		
	public:
		SinkImpl() = default;

	private:
		void fCheck(size_t instance, bool checkIfClosed) const;
		std::unordered_set<std::u8string>::iterator fAllocLabel(std::u8string label);
		void fCloseUntil(size_t size);
		void fPush(const std::u8string_view& name);
		void fPop();
		void fAddLine(const std::u8string_view& str);

	public:
		std::u8string targetString(const text::Target& target);
		size_t nextInstance(std::u8string content, std::vector<wasm::Param> params, std::u8string& out);

	public:
		text::Target pushConditional(size_t instance, const std::u8string_view& label, const text::Prototype* prototype);
		text::Target pushTarget(size_t instance, const std::u8string_view& label, const text::Prototype* prototype, bool block);
		void toggleConditional(text::Target& target);
		void pop(text::Target& target);

		text::Variable getParameter(size_t instance, uint32_t index);
		text::Variable addLocal(size_t instance, wasm::Type type, const std::u8string_view& id);
		void addInst(size_t instance, const text::Instruction& inst);
		void close(size_t instance);
	};

	class SinkWrapper {
	private:
		text::SinkImpl* pSink = 0;
		size_t pInstance = 0;

	public:
		SinkWrapper(text::SinkImpl* sink, size_t instance);
		SinkWrapper(const text::SinkWrapper&) = delete;
		SinkWrapper(text::SinkWrapper&& sink) noexcept;

	public:
		text::Target pushLoop(const std::u8string_view& label, const text::Prototype* prototype);
		void popLoop(text::Target& target);

		text::Target pushBlock(const std::u8string_view& label, const text::Prototype* prototype);
		void popBlock(text::Target& target);

		text::Target pushConditional(const std::u8string_view& label, const text::Prototype* prototype);
		void toggleConditional(text::Target& target);
		void popConditional(text::Target& target);

		text::Variable getParameter(uint32_t index);
		text::Variable addLocal(wasm::Type type, const std::u8string_view& id);
		void addInst(const text::Instruction& inst);
		void close();
	};
}
