/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2024 Bjoern Boss Henrichsen */
#include "text-module.h"
#include "text-sink.h"

wasm::text::Sink::Sink(text::Module* module, std::u8string header) : pModule{ module } {
	pLocals = std::move(header);
	str::BuildTo(pDepth, u8'\n', pModule->pIndent, pModule->pIndent);
}

void wasm::text::Sink::fAddLine(std::u8string_view str) {
	str::BuildTo(pBody, pDepth, str);
}
void wasm::text::Sink::fPush(std::u8string_view name) {
	str::BuildTo(pBody, pDepth, u8'(', name);
	pDepth.append(pModule->pIndent);
}
void wasm::text::Sink::fPop() {
	pDepth.resize(pDepth.size() - pModule->pIndent.size());
	str::BuildTo(pBody, pDepth, u8')');
}

void wasm::text::Sink::pushScope(const wasm::Target& target) {
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
	if (!target.prototype().parameter().empty() || !target.prototype().result().empty())
		text.append(text::MakePrototype(target.prototype()));

	/* push the actual block out */
	fPush(text);
	if (target.type() == wasm::ScopeType::conditional)
		fPush(u8"then");
}
void wasm::text::Sink::popScope(wasm::ScopeType type) {
	fPop();
	if (type == wasm::ScopeType::conditional)
		fPop();
}
void wasm::text::Sink::toggleConditional() {
	fPop();
	fPush(u8"else");
}
void wasm::text::Sink::close(const wasm::Sink& sink) {
	str::BuildTo(pModule->pDefined, pLocals, pBody, u8'\n', pModule->pIndent, u8')');

	/* delete this sink (no reference will be held anymore) */
	delete this;
}
void wasm::text::Sink::addLocal(const wasm::Variable& local) {
	str::BuildTo(pLocals,
		u8'\n', pModule->pIndent, pModule->pIndent, u8"(local",
		text::MakeId(local.id()),
		text::MakeType(local.type()),
		u8')');
}
void wasm::text::Sink::addComment(std::u8string_view text) {
	fAddLine(str::u8::Build(u8"(; ", text, u8" ;)"));
}
void wasm::text::Sink::addInst(const wasm::InstSimple& inst) {
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
		throw wasm::Exception{ L"Unknown wasm::InstSimple type [", size_t(inst.type), L"] encountered" };
	}
}
void wasm::text::Sink::addInst(const wasm::InstConst& inst) {
	std::u8string line;

	/* write the i32 out (as signed/unsigned or hex) */
	if (std::holds_alternative<uint32_t>(inst.value)) {
		uint32_t value = std::get<uint32_t>(inst.value);
		if ((value >> 16) == 0xffff)
			str::BuildTo(line, u8"i32.const ", int32_t(value));
		else if ((value >> 16) != 0)
			str::BuildTo(line, u8"i32.const ", str::As{ U"#x", value });
		else
			str::BuildTo(line, u8"i32.const ", value);
	}

	/* write the i64 out (as signed/unsigned or hex) */
	else if (std::holds_alternative<uint64_t>(inst.value)) {
		uint64_t value = std::get<uint64_t>(inst.value);
		if ((value >> 32) == 0xffff'ffff)
			str::BuildTo(line, u8"i64.const ", int64_t(value));
		else if ((value >> 32) != 0)
			str::BuildTo(line, u8"i64.const ", str::As{ U"#x", value });
		else
			str::BuildTo(line, u8"i64.const ", value);
	}

	/* write the float type directly out */
	else if (std::holds_alternative<float>(inst.value))
		str::BuildTo(line, u8"f32.const ", std::get<float>(inst.value));
	else if (std::holds_alternative<double>(inst.value))
		str::BuildTo(line, u8"f64.const ", std::get<double>(inst.value));
	else
		throw wasm::Exception{ L"Unknown wasm::InstConst type encountered" };

	/* write the line out */
	fAddLine(line);
}
void wasm::text::Sink::addInst(const wasm::InstOperand& inst) {
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
		throw wasm::Exception{ L"Unknown wasm::InstOperand type [", size_t(inst.type), L"] encountered" };
	}

	/* write the line out */
	fAddLine(line);
}
void wasm::text::Sink::addInst(const wasm::InstWidth& inst) {
	std::u8string_view width{ inst.width32 ? u8"32" : u8"64" };

	/* write the general instruction-type out */
	switch (inst.type) {
	case wasm::InstWidth::Type::equalZero:
		fAddLine(str::u8::Build(u8'i', width, u8".eqz"));
		break;
	case wasm::InstWidth::Type::greater:
		fAddLine(str::u8::Build(u8'f', width, u8".gt"));
		break;
	case wasm::InstWidth::Type::less:
		fAddLine(str::u8::Build(u8'f', width, u8".lt"));
		break;
	case wasm::InstWidth::Type::greaterEqual:
		fAddLine(str::u8::Build(u8'f', width, u8".ge"));
		break;
	case wasm::InstWidth::Type::lessEqual:
		fAddLine(str::u8::Build(u8'f', width, u8".le"));
		break;
	case wasm::InstWidth::Type::greaterSigned:
		fAddLine(str::u8::Build(u8'i', width, u8".gt_s"));
		break;
	case wasm::InstWidth::Type::greaterUnsigned:
		fAddLine(str::u8::Build(u8'i', width, u8".gt_u"));
		break;
	case wasm::InstWidth::Type::lessSigned:
		fAddLine(str::u8::Build(u8'i', width, u8".lt_s"));
		break;
	case wasm::InstWidth::Type::lessUnsigned:
		fAddLine(str::u8::Build(u8'i', width, u8".lt_u"));
		break;
	case wasm::InstWidth::Type::greaterEqualSigned:
		fAddLine(str::u8::Build(u8'i', width, u8".ge_s"));
		break;
	case wasm::InstWidth::Type::greaterEqualUnsigned:
		fAddLine(str::u8::Build(u8'i', width, u8".ge_u"));
		break;
	case wasm::InstWidth::Type::lessEqualSigned:
		fAddLine(str::u8::Build(u8'i', width, u8".le_s"));
		break;
	case wasm::InstWidth::Type::lessEqualUnsigned:
		fAddLine(str::u8::Build(u8'i', width, u8".le_u"));
		break;
	case wasm::InstWidth::Type::divSigned:
		fAddLine(str::u8::Build(u8'i', width, u8".div_s"));
		break;
	case wasm::InstWidth::Type::divUnsigned:
		fAddLine(str::u8::Build(u8'i', width, u8".div_u"));
		break;
	case wasm::InstWidth::Type::modSigned:
		fAddLine(str::u8::Build(u8'i', width, u8".rem_s"));
		break;
	case wasm::InstWidth::Type::modUnsigned:
		fAddLine(str::u8::Build(u8'i', width, u8".rem_u"));
		break;
	case wasm::InstWidth::Type::convertToF32Signed:
		fAddLine(str::u8::Build(u8"f32.convert_i", width, u8"_s"));
		break;
	case wasm::InstWidth::Type::convertToF32Unsigned:
		fAddLine(str::u8::Build(u8"f32.convert_i", width, u8"_u"));
		break;
	case wasm::InstWidth::Type::convertToF64Signed:
		fAddLine(str::u8::Build(u8"f64.convert_i", width, u8"_s"));
		break;
	case wasm::InstWidth::Type::convertToF64Unsigned:
		fAddLine(str::u8::Build(u8"f64.convert_i", width, u8"_u"));
		break;
	case wasm::InstWidth::Type::convertFromF32SignedTrap:
		fAddLine(str::u8::Build(u8'i', width, u8".trunc_f32_s"));
		break;
	case wasm::InstWidth::Type::convertFromF32UnsignedTrap:
		fAddLine(str::u8::Build(u8'i', width, u8".trunc_f32_u"));
		break;
	case wasm::InstWidth::Type::convertFromF64SignedTrap:
		fAddLine(str::u8::Build(u8'i', width, u8".trunc_f64_s"));
		break;
	case wasm::InstWidth::Type::convertFromF64UnsignedTrap:
		fAddLine(str::u8::Build(u8'i', width, u8".trunc_f64_u"));
		break;
	case wasm::InstWidth::Type::convertFromF32SignedNoTrap:
		fAddLine(str::u8::Build(u8'i', width, u8".trunc_sat_f32_s"));
		break;
	case wasm::InstWidth::Type::convertFromF32UnsignedNoTrap:
		fAddLine(str::u8::Build(u8'i', width, u8".trunc_sat_f32_u"));
		break;
	case wasm::InstWidth::Type::convertFromF64SignedNoTrap:
		fAddLine(str::u8::Build(u8'i', width, u8".trunc_sat_f64_s"));
		break;
	case wasm::InstWidth::Type::convertFromF64UnsignedNoTrap:
		fAddLine(str::u8::Build(u8'i', width, u8".trunc_sat_f64_u"));
		break;
	case wasm::InstWidth::Type::reinterpretAsFloat:
		fAddLine(str::u8::Build(u8'f', width, u8".reinterpret_i", width));
		break;
	case wasm::InstWidth::Type::bitAnd:
		fAddLine(str::u8::Build(u8'i', width, u8".and"));
		break;
	case wasm::InstWidth::Type::bitOr:
		fAddLine(str::u8::Build(u8'i', width, u8".or"));
		break;
	case wasm::InstWidth::Type::bitXOr:
		fAddLine(str::u8::Build(u8'i', width, u8".xor"));
		break;
	case wasm::InstWidth::Type::bitShiftLeft:
		fAddLine(str::u8::Build(u8'i', width, u8".shl"));
		break;
	case wasm::InstWidth::Type::bitShiftRightSigned:
		fAddLine(str::u8::Build(u8'i', width, u8".shr_s"));
		break;
	case wasm::InstWidth::Type::bitShiftRightUnsigned:
		fAddLine(str::u8::Build(u8'i', width, u8".shr_u"));
		break;
	case wasm::InstWidth::Type::bitRotateLeft:
		fAddLine(str::u8::Build(u8'i', width, u8".rotl"));
		break;
	case wasm::InstWidth::Type::bitRotateRight:
		fAddLine(str::u8::Build(u8'i', width, u8".rotr"));
		break;
	case wasm::InstWidth::Type::bitLeadingNulls:
		fAddLine(str::u8::Build(u8'i', width, u8".clz"));
		break;
	case wasm::InstWidth::Type::bitTrailingNulls:
		fAddLine(str::u8::Build(u8'i', width, u8".ctz"));
		break;
	case wasm::InstWidth::Type::bitSetCount:
		fAddLine(str::u8::Build(u8'i', width, u8".popcnt"));
		break;
	case wasm::InstWidth::Type::floatDiv:
		fAddLine(str::u8::Build(u8'f', width, u8".div"));
		break;
	case wasm::InstWidth::Type::reinterpretAsInt:
		fAddLine(str::u8::Build(u8'i', width, u8".reinterpret_f", width));
		break;
	case wasm::InstWidth::Type::floatMin:
		fAddLine(str::u8::Build(u8'f', width, u8".min"));
		break;
	case wasm::InstWidth::Type::floatMax:
		fAddLine(str::u8::Build(u8'f', width, u8".max"));
		break;
	case wasm::InstWidth::Type::floatFloor:
		fAddLine(str::u8::Build(u8'f', width, u8".floor"));
		break;
	case wasm::InstWidth::Type::floatRound:
		fAddLine(str::u8::Build(u8'f', width, u8".nearest"));
		break;
	case wasm::InstWidth::Type::floatCeil:
		fAddLine(str::u8::Build(u8'f', width, u8".ceil"));
		break;
	case wasm::InstWidth::Type::floatTruncate:
		fAddLine(str::u8::Build(u8'f', width, u8".trunc"));
		break;
	case wasm::InstWidth::Type::floatAbsolute:
		fAddLine(str::u8::Build(u8'f', width, u8".abs"));
		break;
	case wasm::InstWidth::Type::floatNegate:
		fAddLine(str::u8::Build(u8'f', width, u8".neg"));
		break;
	case wasm::InstWidth::Type::floatSquareRoot:
		fAddLine(str::u8::Build(u8'f', width, u8".sqrt"));
		break;
	case wasm::InstWidth::Type::floatCopySign:
		fAddLine(str::u8::Build(u8'f', width, u8".copysign"));
		break;
	default:
		throw wasm::Exception{ L"Unknown wasm::InstWidth type [", size_t(inst.type), L"] encountered" };
	}
}
void wasm::text::Sink::addInst(const wasm::InstMemory& inst) {
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
	case wasm::InstMemory::Type::size:
		line = u8"memory.size";
		break;
	case wasm::InstMemory::Type::grow:
		line = u8"memory.grow";
		break;
	case wasm::InstMemory::Type::copy:
		line = u8"memory.copy";
		break;
	case wasm::InstMemory::Type::fill:
		line = u8"memory.fill";
		break;
	default:
		throw wasm::Exception{ L"Unknown wasm::InstMemory type [", size_t(inst.type), L"] encountered" };
	}

	/* construct the common load/store instruction */
	if (!name.empty())
		str::BuildTo(line, text::MakeOperand(inst.operand), name);

	/* add the memory references (first index indicates destination) */
	if (inst.type == wasm::InstMemory::Type::copy)
		str::BuildTo(line, u8" ", inst.destination.toString());
	str::BuildTo(line, u8" ", inst.memory.toString());

	/* add the offset and write the line out */
	if (!name.empty() && inst.offset > 0)
		str::BuildTo(line, u8" offset=", inst.offset);
	fAddLine(line);
}
void wasm::text::Sink::addInst(const wasm::InstTable& inst) {
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
		throw wasm::Exception{ L"Unknown wasm::InstTable type [", size_t(inst.type), L"] encountered" };
	}

	/* add the table references and write the line out (first memory indicates destination) */
	if (inst.type == wasm::InstTable::Type::copy)
		str::BuildTo(line, u8" ", inst.destination.toString());
	str::BuildTo(line, u8" ", inst.table.toString());
	fAddLine(line);
}
void wasm::text::Sink::addInst(const wasm::InstLocal& inst) {
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
		throw wasm::Exception{ L"Unknown wasm::InstLocal type [", size_t(inst.type), L"] encountered" };
	}

	/* add the variable reference and write the line out */
	line.append(inst.variable.toString());
	fAddLine(line);
}
void wasm::text::Sink::addInst(const wasm::InstGlobal& inst) {
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
		throw wasm::Exception{ L"Unknown wasm::InstGlobal type [", size_t(inst.type), L"] encountered" };
	}

	/* add the global reference and write the line out */
	line.append(inst.global.toString());
	fAddLine(line);
}
void wasm::text::Sink::addInst(const wasm::InstFunction& inst) {
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
		throw wasm::Exception{ L"Unknown wasm::InstFunction type [", size_t(inst.type), L"] encountered" };
	}

	/* add the function reference and write the line out */
	line.append(inst.function.toString());
	fAddLine(line);
}
void wasm::text::Sink::addInst(const wasm::InstIndirect& inst) {
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
		throw wasm::Exception{ L"Unknown wasm::InstIndirect type [", size_t(inst.type), L"] encountered" };
	}

	/* add the table and prototype reference and write the line out */
	line.append(inst.table.toString());
	str::BuildTo(line, u8" (type ", inst.prototype.toString(), u8')');
	fAddLine(line);
}
void wasm::text::Sink::addInst(const wasm::InstBranch& inst) {
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
		throw wasm::Exception{ L"Unknown wasm::InstBranch type [", size_t(inst.type), L"] encountered" };
	}

	/* add the default target and write the line out */
	line.append(inst.target.toString());
	fAddLine(line);
}
