#include "text-module.h"
#include "text-sink.h"

writer::text::Sink::Sink(text::Module* module, std::u8string header) : pModule{ module } {
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
void writer::text::Sink::addInst(const wasm::InstSimple& inst) {
	/* write the general instruction-type out */
	switch (inst.type) {
	case wasm::InstSimple::Type::drop:
		fAddLine(u8"drop");
		break;
	case wasm::InstSimple::Type::nop:
		fAddLine(u8"nop");
		break;
	case wasm::InstSimple::Type::ret:
		fAddLine(u8"return");
		break;
	case wasm::InstSimple::Type::unreachable:
		fAddLine(u8"unreachable");
		break;
	case wasm::InstSimple::Type::select:
		fAddLine(u8"select");
		break;
	case wasm::InstSimple::Type::selectRefFunction:
		fAddLine(u8"select (result funcref)");
		break;
	case wasm::InstSimple::Type::selectRefExtern:
		fAddLine(u8"select (result externref)");
		break;
	case wasm::InstSimple::Type::refTestNull:
		fAddLine(u8"ref.is_null");
		break;
	case wasm::InstSimple::Type::refNullFunction:
		fAddLine(u8"ref.null func");
		break;
	case wasm::InstSimple::Type::refNullExtern:
		fAddLine(u8"ref.null extern");
		break;
	case wasm::InstSimple::Type::expandIntSigned:
		fAddLine(u8"i64.extend_i32_s");
		break;
	case wasm::InstSimple::Type::expandIntUnsigned:
		fAddLine(u8"i64.extend_i32_u");
		break;
	case wasm::InstSimple::Type::shrinkInt:
		fAddLine(u8"i32.wrap_i64");
		break;
	case wasm::InstSimple::Type::expandFloat:
		fAddLine(u8"f64.promote_f32");
		break;
	case wasm::InstSimple::Type::shrinkFloat:
		fAddLine(u8"f32.demote_f64");
		break;
	default:
		util::fail(u8"Unknown wasm::InstSimple type [", size_t(inst.type), u8"] encountered");
		break;
	}
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
void writer::text::Sink::addInst(const wasm::InstOperand& inst) {
	std::u8string line{ text::MakeOperand(inst.operand) };

	/* add the general instruction-type */
	switch (inst.type) {
	case wasm::InstOperand::Type::equal:
		line.append(u8".eq");
		break;
	case wasm::InstOperand::Type::notEqual:
		line.append(u8".ne");
		break;
	case wasm::InstOperand::Type::add:
		line.append(u8".add");
		break;
	case wasm::InstOperand::Type::sub:
		line.append(u8".sub");
		break;
	case wasm::InstOperand::Type::mul:
		line.append(u8".mul");
		break;
	default:
		util::fail(u8"Unknown wasm::InstOperand type [", size_t(inst.type), u8"] encountered");
		break;
	}

	/* write the line out */
	fAddLine(line);
}
void writer::text::Sink::addInst(const wasm::InstWidth& inst) {
	std::u8string_view width{ inst.width32 ? u8"32" : u8"64" };

	/* write the general instruction-type out */
	switch (inst.type) {
	case wasm::InstWidth::Type::equalZero:
		fAddLine(str::Build<std::u8string>(u8'i', width, u8".eqz"));
		break;
	case wasm::InstWidth::Type::greater:
		fAddLine(str::Build<std::u8string>(u8'f', width, u8".gt"));
		break;
	case wasm::InstWidth::Type::less:
		fAddLine(str::Build<std::u8string>(u8'f', width, u8".lt"));
		break;
	case wasm::InstWidth::Type::greaterEqual:
		fAddLine(str::Build<std::u8string>(u8'f', width, u8".ge"));
		break;
	case wasm::InstWidth::Type::lessEqual:
		fAddLine(str::Build<std::u8string>(u8'f', width, u8".le"));
		break;
	case wasm::InstWidth::Type::greaterSigned:
		fAddLine(str::Build<std::u8string>(u8'i', width, u8".gt_s"));
		break;
	case wasm::InstWidth::Type::greaterUnsigned:
		fAddLine(str::Build<std::u8string>(u8'i', width, u8".gt_u"));
		break;
	case wasm::InstWidth::Type::lessSigned:
		fAddLine(str::Build<std::u8string>(u8'i', width, u8".lt_s"));
		break;
	case wasm::InstWidth::Type::lessUnsigned:
		fAddLine(str::Build<std::u8string>(u8'i', width, u8".lt_u"));
		break;
	case wasm::InstWidth::Type::greaterEqualSigned:
		fAddLine(str::Build<std::u8string>(u8'i', width, u8".ge_s"));
		break;
	case wasm::InstWidth::Type::greaterEqualUnsigned:
		fAddLine(str::Build<std::u8string>(u8'i', width, u8".ge_u"));
		break;
	case wasm::InstWidth::Type::lessEqualSigned:
		fAddLine(str::Build<std::u8string>(u8'i', width, u8".le_s"));
		break;
	case wasm::InstWidth::Type::lessEqualUnsigned:
		fAddLine(str::Build<std::u8string>(u8'i', width, u8".le_u"));
		break;
	case wasm::InstWidth::Type::divSigned:
		fAddLine(str::Build<std::u8string>(u8'i', width, u8".div_s"));
		break;
	case wasm::InstWidth::Type::divUnsigned:
		fAddLine(str::Build<std::u8string>(u8'i', width, u8".div_u"));
		break;
	case wasm::InstWidth::Type::modSigned:
		fAddLine(str::Build<std::u8string>(u8'i', width, u8".rem_s"));
		break;
	case wasm::InstWidth::Type::modUnsigned:
		fAddLine(str::Build<std::u8string>(u8'i', width, u8".rem_u"));
		break;
	case wasm::InstWidth::Type::convertToF32Signed:
		fAddLine(str::Build<std::u8string>(u8"f32.convert_i", width, u8"_s"));
		break;
	case wasm::InstWidth::Type::convertToF32Unsigned:
		fAddLine(str::Build<std::u8string>(u8"f32.convert_i", width, u8"_u"));
		break;
	case wasm::InstWidth::Type::convertToF64Signed:
		fAddLine(str::Build<std::u8string>(u8"f64.convert_i", width, u8"_s"));
		break;
	case wasm::InstWidth::Type::convertToF64Unsigned:
		fAddLine(str::Build<std::u8string>(u8"f64.convert_i", width, u8"_u"));
		break;
	case wasm::InstWidth::Type::convertFromF32Signed:
		fAddLine(str::Build<std::u8string>(u8'i', width, u8".trunc_f32_s"));
		break;
	case wasm::InstWidth::Type::convertFromF32Unsigned:
		fAddLine(str::Build<std::u8string>(u8'i', width, u8".trunc_f32_u"));
		break;
	case wasm::InstWidth::Type::convertFromF64Signed:
		fAddLine(str::Build<std::u8string>(u8'i', width, u8".trunc_f64_s"));
		break;
	case wasm::InstWidth::Type::convertFromF64Unsigned:
		fAddLine(str::Build<std::u8string>(u8'i', width, u8".trunc_f64_u"));
		break;
	case wasm::InstWidth::Type::reinterpretAsFloat:
		fAddLine(str::Build<std::u8string>(u8'i', width, u8".reinterpret_f", width));
		break;
	case wasm::InstWidth::Type::bitAnd:
		fAddLine(str::Build<std::u8string>(u8'i', width, u8".and"));
		break;
	case wasm::InstWidth::Type::bitOr:
		fAddLine(str::Build<std::u8string>(u8'i', width, u8".or"));
		break;
	case wasm::InstWidth::Type::bitXOr:
		fAddLine(str::Build<std::u8string>(u8'i', width, u8".xor"));
		break;
	case wasm::InstWidth::Type::bitShiftLeft:
		fAddLine(str::Build<std::u8string>(u8'i', width, u8".shl"));
		break;
	case wasm::InstWidth::Type::bitShiftRightSigned:
		fAddLine(str::Build<std::u8string>(u8'i', width, u8".shr_s"));
		break;
	case wasm::InstWidth::Type::bitShiftRightUnsigned:
		fAddLine(str::Build<std::u8string>(u8'i', width, u8".shr_u"));
		break;
	case wasm::InstWidth::Type::bitRotateLeft:
		fAddLine(str::Build<std::u8string>(u8'i', width, u8".rotl"));
		break;
	case wasm::InstWidth::Type::bitRotateRight:
		fAddLine(str::Build<std::u8string>(u8'i', width, u8".rotr"));
		break;
	case wasm::InstWidth::Type::bitLeadingNulls:
		fAddLine(str::Build<std::u8string>(u8'i', width, u8".clz"));
		break;
	case wasm::InstWidth::Type::bitTrailingNulls:
		fAddLine(str::Build<std::u8string>(u8'i', width, u8".ctz"));
		break;
	case wasm::InstWidth::Type::bitSetCount:
		fAddLine(str::Build<std::u8string>(u8'i', width, u8".popcnt"));
		break;
	case wasm::InstWidth::Type::floatDiv:
		fAddLine(str::Build<std::u8string>(u8'f', width, u8".div"));
		break;
	case wasm::InstWidth::Type::reinterpretAsInt:
		fAddLine(str::Build<std::u8string>(u8'f', width, u8".reinterpret_i", width));
		break;
	case wasm::InstWidth::Type::floatMin:
		fAddLine(str::Build<std::u8string>(u8'f', width, u8".min"));
		break;
	case wasm::InstWidth::Type::floatMax:
		fAddLine(str::Build<std::u8string>(u8'f', width, u8".max"));
		break;
	case wasm::InstWidth::Type::floatFloor:
		fAddLine(str::Build<std::u8string>(u8'f', width, u8".floor"));
		break;
	case wasm::InstWidth::Type::floatRound:
		fAddLine(str::Build<std::u8string>(u8'f', width, u8".nearest"));
		break;
	case wasm::InstWidth::Type::floatCeil:
		fAddLine(str::Build<std::u8string>(u8'f', width, u8".ceil"));
		break;
	case wasm::InstWidth::Type::floatTruncate:
		fAddLine(str::Build<std::u8string>(u8'f', width, u8".trunc"));
		break;
	case wasm::InstWidth::Type::floatAbsolute:
		fAddLine(str::Build<std::u8string>(u8'f', width, u8".abs"));
		break;
	case wasm::InstWidth::Type::floatNegate:
		fAddLine(str::Build<std::u8string>(u8'f', width, u8".neg"));
		break;
	case wasm::InstWidth::Type::floatSquareRoot:
		fAddLine(str::Build<std::u8string>(u8'f', width, u8".sqrt"));
		break;
	case wasm::InstWidth::Type::floatCopySign:
		fAddLine(str::Build<std::u8string>(u8'f', width, u8".copysign"));
		break;
	default:
		util::fail(u8"Unknown wasm::InstWidth type [", size_t(inst.type), u8"] encountered");
		break;
	}
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
	if (!name.empty())
		str::BuildTo(line, text::MakeOperand(inst.operand), name);

	/* add the memory references */
	str::BuildTo(line, u8" ", inst.memory.toString());
	if (inst.type == wasm::InstMemory::Type::copy)
		str::BuildTo(line, u8" ", inst.destination.toString());

	/* add the offset and write the line out */
	if (!name.empty() && inst.offset > 0)
		str::BuildTo(line, u8" offset=", inst.offset);
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
	str::BuildTo(line, u8" ", inst.table.toString());
	if (inst.type == wasm::InstTable::Type::copy)
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
	str::BuildTo(line, u8" (table ", inst.table.toString(), u8')');
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
