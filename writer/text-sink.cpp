#include "text-sink.h"
#include "text-module.h"

writer::text::Sink::Sink(text::Module* module, std::u8string header) {
	pModule = module;
	pLocals = std::move(header);
	pDepth = u8"\n    ";
}

void writer::text::Sink::fAddLine(const std::u8string_view& str) {
	str::BuildTo(pBody, pDepth, str);
}
void writer::text::Sink::fPush(const std::u8string_view& name) {
	str::BuildTo(pBody, pDepth, u8'(', name);
	pDepth.append(u8"  ");
}
void writer::text::Sink::fPop() {
	pDepth.resize(pDepth.size() - 2);
	str::BuildTo(pBody, pDepth, u8')');
}

std::u8string_view writer::text::Sink::fOperand(wasm::OpType operand) const {
	switch (operand) {
	case wasm::OpType::i32:
		return u8"i32";
	case wasm::OpType::i64:
		return u8"i64";
	case wasm::OpType::f32:
		return u8"f32";
	case wasm::OpType::f64:
		return u8"f64";
	default:
		util::fail(u8"Unknown operand type [", size_t(operand), u8"] encountered");
		return u8"";
	}
}

void writer::text::Sink::pushScope(const wasm::Target& target) {
	std::u8string text;

	/* fetch the block-type */
	if (target.type() == wasm::ScopeType::conditional)
		text = u8"if";
	else if (target.type() == wasm::ScopeType::loop)
		text = u8"loop";
	else
		text = u8"block";

	/* add the id and the prototype */
	text.append(text::MakeId(target.id()));
	text.append(text::MakePrototype(target.prototype()));

	/* push the actual block out */
	fPush(text);
	if (target.type() == wasm::ScopeType::conditional)
		fPush(u8"then");
}
void writer::text::Sink::popScope(wasm::ScopeType type) {
	fPop();
	if (type == wasm::ScopeType::conditional)
		fPop();
}
void writer::text::Sink::toggleConditional() {
	fPop();
	fPush(u8"else");
}
void writer::text::Sink::close(const wasm::Sink& sink) {
	str::BuildTo(pModule->pDefined, pLocals, pBody, u8"\n  )");
	delete this;
}
void writer::text::Sink::addLocal(const wasm::Variable& local) {
	str::BuildTo(pLocals,
		u8"\n    (local",
		text::MakeId(local.id()),
		text::MakeType(local.type()),
		u8')');
}
void writer::text::Sink::addInst(const wasm::InstConst& inst) {
	fAddLine(u8"wasm::InstConst");
}
void writer::text::Sink::addInst(const wasm::InstSimple& inst) {
	fAddLine(u8"wasm::InstSimple");
}
void writer::text::Sink::addInst(const wasm::InstMemory& inst) {
	std::u8string_view name;
	std::u8string line;

	/* add the general instruction-type */
	switch (inst.type) {
	case wasm::InstMemory::Type::load:
		name = u8".load";
		break;
	case wasm::InstMemory::Type::load8Unsigned:
		name = u8".load8_u";
		break;
	case wasm::InstMemory::Type::load8Signed:
		name = u8".load8_s";
		break;
	case wasm::InstMemory::Type::load16Unsigned:
		name = u8".load16_u";
		break;
	case wasm::InstMemory::Type::load16Signed:
		name = u8".load16_s";
		break;
	case wasm::InstMemory::Type::load32Unsigned:
		name = u8".load32_u";
		break;
	case wasm::InstMemory::Type::load32Signed:
		name = u8".load32_s";
		break;
	case wasm::InstMemory::Type::store:
		name = u8".store";
		break;
	case wasm::InstMemory::Type::store8:
		name = u8".store8";
		break;
	case wasm::InstMemory::Type::store16:
		name = u8".store16";
		break;
	case wasm::InstMemory::Type::store32:
		name = u8".store32";
		break;
	case wasm::InstMemory::Type::grow:
		line = u8"memory.grow";
		break;
	case wasm::InstMemory::Type::size:
		line = u8"memory.size";
		break;
	case wasm::InstMemory::Type::copy:
		line = u8"memory.copy";
		break;
	case wasm::InstMemory::Type::fill:
		line = u8"memory.fill";
		break;
	default:
		util::fail(u8"Unknown wasm::InstMemory type [", size_t(inst.type), u8"] encountered");
		break;
	}

	/* construct the common load/store instruction */
	if (!name.empty()) {
		str::BuildTo(line, fOperand(inst.operand), name);
		if (inst.offset > 0)
			str::BuildTo(line, u8" offset=", inst.offset);
	}

	/* add the memory references */
	if (inst.memory.valid())
		str::BuildTo(line, u8" (memory ", inst.memory.toString(), u8')');
	if (inst.type == wasm::InstMemory::Type::copy && inst.destination.valid())
		str::BuildTo(line, u8" (memory ", inst.destination.toString(), u8')');

	/* write the memory-line out */
	fAddLine(line);
}
void writer::text::Sink::addInst(const wasm::InstTable& inst) {
	fAddLine(u8"wasm::InstTable");
}
void writer::text::Sink::addInst(const wasm::InstLocal& inst) {
	fAddLine(u8"wasm::InstLocal");
}
void writer::text::Sink::addInst(const wasm::InstGlobal& inst) {
	fAddLine(u8"wasm::InstGlobal");
}
void writer::text::Sink::addInst(const wasm::InstFunction& inst) {
	fAddLine(u8"wasm::InstFunction");
}
void writer::text::Sink::addInst(const wasm::InstIndirect& inst) {
	fAddLine(u8"wasm::InstIndirect");
}
void writer::text::Sink::addInst(const wasm::InstBranch& inst) {
	std::u8string line;

	/* add the general instruction-type */
	switch (inst.type) {
	case wasm::InstBranch::Type::direct:
		line.append(u8"br ");
		break;
	case wasm::InstBranch::Type::conditional:
		line.append(u8"br_if ");
		break;
	case wasm::InstBranch::Type::table:
		line.append(u8"br_table ");
		for (size_t i = 0; i < inst.list.size(); ++i)
			str::BuildTo(line, inst.list.begin()[i].get().toString(), u8' ');
		break;
	default:
		util::fail(u8"Unknown wasm::InstBranch type [", size_t(inst.type), u8"] encountered");
		break;
	}

	/* add the default target and write the line out */
	line.append(inst.target.toString());
	fAddLine(line);
}
