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
	std::u8string line;

	/* construct the constant-line */
	if (std::holds_alternative<uint32_t>(inst.value))
		str::BuildTo(line, u8"i32.const ", std::get<uint32_t>(inst.value));
	else if (std::holds_alternative<uint64_t>(inst.value))
		str::BuildTo(line, u8"i64.const ", std::get<uint64_t>(inst.value));
	else if (std::holds_alternative<float>(inst.value))
		str::BuildTo(line, u8"f32.const ", std::get<float>(inst.value));
	else if (std::holds_alternative<double>(inst.value))
		str::BuildTo(line, u8"f64.const ", std::get<double>(inst.value));
	else
		util::fail(u8"Unknown wasm::InstConst type encountered");

	/* write the line out */
	fAddLine(line);
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

	/* add the memory references and write the line out */
	if (inst.memory.valid())
		str::BuildTo(line, u8" ", inst.memory.toString());
	if (inst.type == wasm::InstMemory::Type::copy && inst.destination.valid())
		str::BuildTo(line, u8" ", inst.destination.toString());
	fAddLine(line);
}
void writer::text::Sink::addInst(const wasm::InstTable& inst) {
	std::u8string line;

	/* fetch the general instruction-type */
	switch (inst.type) {
	case wasm::InstTable::Type::get:
		line = u8"table.get";
		break;
	case wasm::InstTable::Type::set:
		line = u8"table.set";
		break;
	case wasm::InstTable::Type::size:
		line = u8"table.size";
		break;
	case wasm::InstTable::Type::grow:
		line = u8"table.grow";
		break;
	case wasm::InstTable::Type::copy:
		line = u8"table.copy";
		break;
	case wasm::InstTable::Type::fill:
		line = u8"table.fill";
		break;
	default:
		util::fail(u8"Unknown wasm::InstTable type [", size_t(inst.type), u8"] encountered");
		break;
	}

	/* add the table references and write the line out */
	if (inst.table.valid())
		str::BuildTo(line, u8" ", inst.table.toString());
	if (inst.type == wasm::InstTable::Type::copy && inst.destination.valid())
		str::BuildTo(line, u8" ", inst.destination.toString());
	fAddLine(line);
}
void writer::text::Sink::addInst(const wasm::InstLocal& inst) {
	std::u8string line;

	/* fetch the general instruction-type */
	switch (inst.type) {
	case wasm::InstLocal::Type::get:
		line = u8"local.get ";
		break;
	case wasm::InstLocal::Type::set:
		line = u8"local.set ";
		break;
	case wasm::InstLocal::Type::tee:
		line = u8"local.tee ";
		break;
	default:
		util::fail(u8"Unknown wasm::InstLocal type [", size_t(inst.type), u8"] encountered");
		break;
	}

	/* add the variable reference and write the line out */
	line.append(inst.variable.toString());
	fAddLine(line);
}
void writer::text::Sink::addInst(const wasm::InstGlobal& inst) {
	std::u8string line;

	/* fetch the general instruction-type */
	switch (inst.type) {
	case wasm::InstGlobal::Type::get:
		line = u8"global.get ";
		break;
	case wasm::InstGlobal::Type::set:
		line = u8"global.set ";
		break;
	default:
		util::fail(u8"Unknown wasm::InstGlobal type [", size_t(inst.type), u8"] encountered");
		break;
	}

	/* add the global reference and write the line out */
	line.append(inst.global.toString());
	fAddLine(line);
}
void writer::text::Sink::addInst(const wasm::InstFunction& inst) {
	std::u8string line;

	/* fetch the general instruction-type */
	switch (inst.type) {
	case wasm::InstFunction::Type::refFunction:
		line = u8"ref.func ";
		break;
	case wasm::InstFunction::Type::callNormal:
		line = u8"call ";
		break;
	case wasm::InstFunction::Type::callTail:
		line = u8"return_call ";
		break;
	default:
		util::fail(u8"Unknown wasm::InstFunction type [", size_t(inst.type), u8"] encountered");
		break;
	}

	/* add the function reference and write the line out */
	line.append(inst.function.toString());
	fAddLine(line);
}
void writer::text::Sink::addInst(const wasm::InstIndirect& inst) {
	std::u8string line;

	/* fetch the general instruction-type */
	switch (inst.type) {
	case wasm::InstIndirect::Type::callNormal:
		line = u8"call_indirect ";
		break;
	case wasm::InstIndirect::Type::callTail:
		line = u8"return_call_indirect ";
		break;
	default:
		util::fail(u8"Unknown wasm::InstIndirect type [", size_t(inst.type), u8"] encountered");
		break;
	}

	/* add the table and prototype reference and write the line out */
	if (inst.table.valid())
		str::BuildTo(line, u8" (table ", inst.table.toString(), u8')');
	if (inst.prototype.valid())
		str::BuildTo(line, u8" (type ", inst.prototype.toString(), u8')');
	fAddLine(line);
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
