/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2024 Bjoern Boss Henrichsen */
#include "binary-module.h"
#include "binary-sink.h"

writer::binary::Sink::Sink(binary::Module* module, uint32_t index) : pModule{ module }, pIndex{ index } {}

void writer::binary::Sink::fPush(uint8_t byte) {
	pCode.push_back(byte);
}
void writer::binary::Sink::fPush(std::initializer_list<uint8_t> bytes) {
	pCode.insert(pCode.end(), bytes.begin(), bytes.end());
}
void writer::binary::Sink::fPushWidth(bool _32, uint8_t i32, uint8_t i64) {
	fPush(_32 ? i32 : i64);
}
void writer::binary::Sink::fPushSelect(wasm::OpType operand, uint8_t i32, uint8_t i64, uint8_t f32, uint8_t f64) {
	switch (operand) {
	case wasm::OpType::i32:
		fPush(i32);
		break;
	case wasm::OpType::i64:
		fPush(i64);
		break;
	case wasm::OpType::f32:
		fPush(f32);
		break;
	case wasm::OpType::f64:
		fPush(f64);
		break;
	default:
		throw wasm::Exception{ L"Unknown operand type [", size_t(operand), L"] encountered" };
		break;
	}
}

void writer::binary::Sink::pushScope(const wasm::Target& target) {
	/* write the block-instruction out */
	if (target.type() == wasm::ScopeType::conditional)
		fPush(0x04);
	else if (target.type() == wasm::ScopeType::loop)
		fPush(0x03);
	else
		fPush(0x02);

	/* write the blocktype out */
	if (target.prototype().parameter().empty() && target.prototype().result().empty())
		fPush(0x40);
	else if (target.prototype().parameter().empty() && target.prototype().result().size() == 1)
		fPush(binary::GetType(target.prototype().result()[0]));
	else
		binary::WriteSInt(pCode, target.prototype().index());
}
void writer::binary::Sink::popScope(wasm::ScopeType type) {
	fPush(0x0b);
}
void writer::binary::Sink::toggleConditional() {
	fPush(0x05);
}
void writer::binary::Sink::close(const wasm::Sink& sink) {
	std::vector<uint8_t>& buffer = pModule->pCode[pIndex];

	/* write the local data to the buffer */
	binary::WriteUInt(buffer, pLocals.size());
	for (size_t i = 0; i < pLocals.size(); ++i) {
		binary::WriteUInt(buffer, pLocals[i].count);
		buffer.push_back(binary::GetType(pLocals[i].type));
	}

	/* write the expression to the buffer */
	buffer.insert(buffer.end(), pCode.begin(), pCode.end());

	/* write the closing instruction-byte */
	buffer.push_back(0x0b);

	/* delete this sink (no reference will be held anymore) */
	delete this;
}
void writer::binary::Sink::addLocal(const wasm::Variable& local) {
	if (pLocals.empty() || pLocals.back().type != local.type())
		pLocals.push_back({ 1, local.type() });
	else
		++pLocals.back().count;
}
void writer::binary::Sink::addInst(const wasm::InstSimple& inst) {
	/* write the general instruction-type out */
	switch (inst.type) {
	case wasm::InstSimple::Type::drop:
		fPush(0x1a);
		break;
	case wasm::InstSimple::Type::nop:
		fPush(0x01);
		break;
	case wasm::InstSimple::Type::ret:
		fPush(0x0f);
		break;
	case wasm::InstSimple::Type::unreachable:
		fPush(0x00);
		break;
	case wasm::InstSimple::Type::select:
		fPush(0x1b);
		break;
	case wasm::InstSimple::Type::selectRefFunction:
		fPush({ 0x1c, 0x01, binary::GetType(wasm::Type::refFunction) });
		break;
	case wasm::InstSimple::Type::selectRefExtern:
		fPush({ 0x1c, 0x01, binary::GetType(wasm::Type::refExtern) });
		break;
	case wasm::InstSimple::Type::refTestNull:
		fPush(0xd1);
		break;
	case wasm::InstSimple::Type::refNullFunction:
		fPush({ 0xd0, binary::GetType(wasm::Type::refFunction) });
		break;
	case wasm::InstSimple::Type::refNullExtern:
		fPush({ 0xd0, binary::GetType(wasm::Type::refExtern) });
		break;
	case wasm::InstSimple::Type::expandIntSigned:
		fPush(0xac);
		break;
	case wasm::InstSimple::Type::expandIntUnsigned:
		fPush(0xad);
		break;
	case wasm::InstSimple::Type::shrinkInt:
		fPush(0xa7);
		break;
	case wasm::InstSimple::Type::expandFloat:
		fPush(0xbb);
		break;
	case wasm::InstSimple::Type::shrinkFloat:
		fPush(0xb6);
		break;
	default:
		throw wasm::Exception{ L"Unknown wasm::InstSimple type [", size_t(inst.type), L"] encountered" };
		break;
	}
}
void writer::binary::Sink::addInst(const wasm::InstConst& inst) {
	if (std::holds_alternative<uint32_t>(inst.value)) {
		fPush(0x41);
		binary::WriteInt32(pCode, std::get<uint32_t>(inst.value));
	}
	else if (std::holds_alternative<uint64_t>(inst.value)) {
		fPush(0x42);
		binary::WriteInt64(pCode, std::get<uint64_t>(inst.value));
	}
	else if (std::holds_alternative<float>(inst.value)) {
		fPush(0x43);
		binary::WriteFloat(pCode, std::get<float>(inst.value));
	}
	else if (std::holds_alternative<double>(inst.value)) {
		fPush(0x44);
		binary::WriteDouble(pCode, std::get<double>(inst.value));
	}
	else
		throw wasm::Exception{ L"Unknown wasm::InstConst type encountered" };
}
void writer::binary::Sink::addInst(const wasm::InstOperand& inst) {
	/* write the instruction out */
	switch (inst.type) {
	case wasm::InstOperand::Type::equal:
		fPushSelect(inst.operand, 0x46, 0x51, 0x5b, 0x61);
		break;
	case wasm::InstOperand::Type::notEqual:
		fPushSelect(inst.operand, 0x47, 0x52, 0x5c, 0x62);
		break;
	case wasm::InstOperand::Type::add:
		fPushSelect(inst.operand, 0x6a, 0x7c, 0x92, 0xa0);
		break;
	case wasm::InstOperand::Type::sub:
		fPushSelect(inst.operand, 0x6b, 0x7d, 0x93, 0xa1);
		break;
	case wasm::InstOperand::Type::mul:
		fPushSelect(inst.operand, 0x6c, 0x7e, 0x94, 0xa2);
		break;
	default:
		throw wasm::Exception{ L"Unknown wasm::InstOperand type [", size_t(inst.type), L"] encountered" };
		break;
	}
}
void writer::binary::Sink::addInst(const wasm::InstWidth& inst) {
	/* write the instruction out */
	switch (inst.type) {
	case wasm::InstWidth::Type::equalZero:
		fPushWidth(inst.width32, 0x45, 0x50);
		break;
	case wasm::InstWidth::Type::greater:
		fPushWidth(inst.width32, 0x5e, 0x64);
		break;
	case wasm::InstWidth::Type::less:
		fPushWidth(inst.width32, 0x5d, 0x63);
		break;
	case wasm::InstWidth::Type::greaterEqual:
		fPushWidth(inst.width32, 0x60, 0x66);
		break;
	case wasm::InstWidth::Type::lessEqual:
		fPushWidth(inst.width32, 0x5f, 0x65);
		break;
	case wasm::InstWidth::Type::greaterSigned:
		fPushWidth(inst.width32, 0x4a, 0x55);
		break;
	case wasm::InstWidth::Type::greaterUnsigned:
		fPushWidth(inst.width32, 0x4b, 0x56);
		break;
	case wasm::InstWidth::Type::lessSigned:
		fPushWidth(inst.width32, 0x48, 0x53);
		break;
	case wasm::InstWidth::Type::lessUnsigned:
		fPushWidth(inst.width32, 0x49, 0x54);
		break;
	case wasm::InstWidth::Type::greaterEqualSigned:
		fPushWidth(inst.width32, 0x4e, 0x59);
		break;
	case wasm::InstWidth::Type::greaterEqualUnsigned:
		fPushWidth(inst.width32, 0x4f, 0x5a);
		break;
	case wasm::InstWidth::Type::lessEqualSigned:
		fPushWidth(inst.width32, 0x4c, 0x57);
		break;
	case wasm::InstWidth::Type::lessEqualUnsigned:
		fPushWidth(inst.width32, 0x4d, 0x58);
		break;
	case wasm::InstWidth::Type::divSigned:
		fPushWidth(inst.width32, 0x6d, 0x7f);
		break;
	case wasm::InstWidth::Type::divUnsigned:
		fPushWidth(inst.width32, 0x6e, 0x80);
		break;
	case wasm::InstWidth::Type::modSigned:
		fPushWidth(inst.width32, 0x6f, 0x81);
		break;
	case wasm::InstWidth::Type::modUnsigned:
		fPushWidth(inst.width32, 0x70, 0x82);
		break;
	case wasm::InstWidth::Type::convertToF32Signed:
		fPushWidth(inst.width32, 0xb2, 0xb4);
		break;
	case wasm::InstWidth::Type::convertToF32Unsigned:
		fPushWidth(inst.width32, 0xb3, 0xb5);
		break;
	case wasm::InstWidth::Type::convertToF64Signed:
		fPushWidth(inst.width32, 0xb7, 0xb9);
		break;
	case wasm::InstWidth::Type::convertToF64Unsigned:
		fPushWidth(inst.width32, 0xb8, 0xba);
		break;
	case wasm::InstWidth::Type::convertFromF32Signed:
		fPushWidth(inst.width32, 0xa8, 0xae);
		break;
	case wasm::InstWidth::Type::convertFromF32Unsigned:
		fPushWidth(inst.width32, 0xa9, 0xaf);
		break;
	case wasm::InstWidth::Type::convertFromF64Signed:
		fPushWidth(inst.width32, 0xaa, 0xb0);
		break;
	case wasm::InstWidth::Type::convertFromF64Unsigned:
		fPushWidth(inst.width32, 0xab, 0xb1);
		break;
	case wasm::InstWidth::Type::reinterpretAsFloat:
		fPushWidth(inst.width32, 0xbc, 0xbd);
		break;
	case wasm::InstWidth::Type::bitAnd:
		fPushWidth(inst.width32, 0x71, 0x83);
		break;
	case wasm::InstWidth::Type::bitOr:
		fPushWidth(inst.width32, 0x72, 0x84);
		break;
	case wasm::InstWidth::Type::bitXOr:
		fPushWidth(inst.width32, 0x73, 0x85);
		break;
	case wasm::InstWidth::Type::bitShiftLeft:
		fPushWidth(inst.width32, 0x74, 0x86);
		break;
	case wasm::InstWidth::Type::bitShiftRightSigned:
		fPushWidth(inst.width32, 0x75, 0x87);
		break;
	case wasm::InstWidth::Type::bitShiftRightUnsigned:
		fPushWidth(inst.width32, 0x76, 0x88);
		break;
	case wasm::InstWidth::Type::bitRotateLeft:
		fPushWidth(inst.width32, 0x77, 0x89);
		break;
	case wasm::InstWidth::Type::bitRotateRight:
		fPushWidth(inst.width32, 0x78, 0x8a);
		break;
	case wasm::InstWidth::Type::bitLeadingNulls:
		fPushWidth(inst.width32, 0x67, 0x79);
		break;
	case wasm::InstWidth::Type::bitTrailingNulls:
		fPushWidth(inst.width32, 0x68, 0x7a);
		break;
	case wasm::InstWidth::Type::bitSetCount:
		fPushWidth(inst.width32, 0x69, 0x7b);
		break;
	case wasm::InstWidth::Type::floatDiv:
		fPushWidth(inst.width32, 0x95, 0xa3);
		break;
	case wasm::InstWidth::Type::reinterpretAsInt:
		fPushWidth(inst.width32, 0xbe, 0xbf);
		break;
	case wasm::InstWidth::Type::floatMin:
		fPushWidth(inst.width32, 0x96, 0xa4);
		break;
	case wasm::InstWidth::Type::floatMax:
		fPushWidth(inst.width32, 0x97, 0xa5);
		break;
	case wasm::InstWidth::Type::floatFloor:
		fPushWidth(inst.width32, 0x8e, 0x9c);
		break;
	case wasm::InstWidth::Type::floatRound:
		fPushWidth(inst.width32, 0x90, 0x9e);
		break;
	case wasm::InstWidth::Type::floatCeil:
		fPushWidth(inst.width32, 0x8d, 0x9b);
		break;
	case wasm::InstWidth::Type::floatTruncate:
		fPushWidth(inst.width32, 0x8f, 0x9d);
		break;
	case wasm::InstWidth::Type::floatAbsolute:
		fPushWidth(inst.width32, 0x8b, 0x99);
		break;
	case wasm::InstWidth::Type::floatNegate:
		fPushWidth(inst.width32, 0x8c, 0x9a);
		break;
	case wasm::InstWidth::Type::floatSquareRoot:
		fPushWidth(inst.width32, 0x91, 0x9f);
		break;
	case wasm::InstWidth::Type::floatCopySign:
		fPushWidth(inst.width32, 0x98, 0xa6);
		break;
	default:
		throw wasm::Exception{ L"Unknown wasm::InstWidth type [", size_t(inst.type), L"] encountered" };
		break;
	}
}
void writer::binary::Sink::addInst(const wasm::InstMemory& inst) {
	bool writeMemoryAndOffset = false;

	/* write the general instruction opcode out */
	switch (inst.type) {
	case wasm::InstMemory::Type::load:
		fPushSelect(inst.operand, 0x28, 0x29, 0x2a, 0x2b);
		writeMemoryAndOffset = true;
		break;
	case wasm::InstMemory::Type::load8Unsigned:
		fPushWidth(inst.operand == wasm::OpType::i32, 0x2d, 0x31);
		writeMemoryAndOffset = true;
		break;
	case wasm::InstMemory::Type::load8Signed:
		fPushWidth(inst.operand == wasm::OpType::i32, 0x2c, 0x30);
		writeMemoryAndOffset = true;
		break;
	case wasm::InstMemory::Type::load16Unsigned:
		fPushWidth(inst.operand == wasm::OpType::i32, 0x2f, 0x33);
		writeMemoryAndOffset = true;
		break;
	case wasm::InstMemory::Type::load16Signed:
		fPushWidth(inst.operand == wasm::OpType::i32, 0x2e, 0x32);
		writeMemoryAndOffset = true;
		break;
	case wasm::InstMemory::Type::load32Unsigned:
		fPush(0x35);
		writeMemoryAndOffset = true;
		break;
	case wasm::InstMemory::Type::load32Signed:
		fPush(0x34);
		writeMemoryAndOffset = true;
		break;
	case wasm::InstMemory::Type::store:
		fPushSelect(inst.operand, 0x36, 0x37, 0x38, 0x39);
		writeMemoryAndOffset = true;
		break;
	case wasm::InstMemory::Type::store8:
		fPushWidth(inst.operand == wasm::OpType::i32, 0x3a, 0x3c);
		writeMemoryAndOffset = true;
		break;
	case wasm::InstMemory::Type::store16:
		fPushWidth(inst.operand == wasm::OpType::i32, 0x3b, 0x3d);
		writeMemoryAndOffset = true;
		break;
	case wasm::InstMemory::Type::store32:
		fPush(0x3e);
		writeMemoryAndOffset = true;
		break;
	case wasm::InstMemory::Type::grow:
		fPush(0x40);
		binary::WriteUInt(pCode, inst.memory.index());
		break;
	case wasm::InstMemory::Type::size:
		fPush(0x3f);
		binary::WriteUInt(pCode, inst.memory.index());
		break;
	case wasm::InstMemory::Type::copy:
		fPush({ 0xfc, 0x0a });
		binary::WriteUInt(pCode, inst.memory.index());
		binary::WriteUInt(pCode, inst.destination.index());
		break;
	case wasm::InstMemory::Type::fill:
		fPush({ 0xfc, 0x0b });
		binary::WriteUInt(pCode, inst.memory.index());
		break;
	default:
		throw wasm::Exception{ L"Unknown wasm::InstMemory type [", size_t(inst.type), L"] encountered" };
		break;
	}

	/* check if the alignment and offset needs to be written out (alignment used to encode multi-memory) */
	if (writeMemoryAndOffset) {
		if (inst.memory.index() != 0) {
			fPush(0x40);
			binary::WriteUInt(pCode, inst.memory.index());
		}
		else
			fPush(0x00);
		binary::WriteUInt(pCode, inst.offset);
	}
}
void writer::binary::Sink::addInst(const wasm::InstTable& inst) {
	/* write the general instruction opcode out */
	switch (inst.type) {
	case wasm::InstTable::Type::get:
		fPush(0x25);
		break;
	case wasm::InstTable::Type::set:
		fPush(0x26);
		break;
	case wasm::InstTable::Type::size:
		fPush({ 0xfc, 0x10 });
		break;
	case wasm::InstTable::Type::grow:
		fPush({ 0xfc, 0x0f });
		break;
	case wasm::InstTable::Type::copy:
		fPush({ 0xfc, 0x0e });
		break;
	case wasm::InstTable::Type::fill:
		fPush({ 0xfc, 0x11 });
		break;
	default:
		throw wasm::Exception{ L"Unknown wasm::InstTable type [", size_t(inst.type), L"] encountered" };
		break;
	}

	/* write the table index out */
	binary::WriteUInt(pCode, inst.table.index());

	/* check if the destination needs to be written out as well */
	if (inst.type == wasm::InstTable::Type::copy)
		binary::WriteUInt(pCode, inst.destination.index());
}
void writer::binary::Sink::addInst(const wasm::InstLocal& inst) {
	/* write the general instruction opcode out */
	switch (inst.type) {
	case wasm::InstLocal::Type::get:
		fPush(0x20);
		break;
	case wasm::InstLocal::Type::set:
		fPush(0x21);
		break;
	case wasm::InstLocal::Type::tee:
		fPush(0x22);
		break;
	default:
		throw wasm::Exception{ L"Unknown wasm::InstLocal type [", size_t(inst.type), L"] encountered" };
		break;
	}

	/* write the local index out */
	binary::WriteUInt(pCode, inst.variable.index());
}
void writer::binary::Sink::addInst(const wasm::InstGlobal& inst) {
	/* write the general instruction opcode out */
	switch (inst.type) {
	case wasm::InstGlobal::Type::get:
		fPush(0x23);
		break;
	case wasm::InstGlobal::Type::set:
		fPush(0x24);
		break;
	default:
		throw wasm::Exception{ L"Unknown wasm::InstGlobal type [", size_t(inst.type), L"] encountered" };
		break;
	}

	/* write the global index out */
	binary::WriteUInt(pCode, inst.global.index());
}
void writer::binary::Sink::addInst(const wasm::InstFunction& inst) {
	/* write the general instruction opcode out */
	switch (inst.type) {
	case wasm::InstFunction::Type::refFunction:
		fPush(0xd2);
		break;
	case wasm::InstFunction::Type::callNormal:
		fPush(0x10);
		break;
	case wasm::InstFunction::Type::callTail:
		fPush(0x12);
		break;
	default:
		throw wasm::Exception{ L"Unknown wasm::InstFunction type [", size_t(inst.type), L"] encountered" };
		break;
	}

	/* write the function index out */
	binary::WriteUInt(pCode, inst.function.index());
}
void writer::binary::Sink::addInst(const wasm::InstIndirect& inst) {
	/* write the general instruction opcode out */
	switch (inst.type) {
	case wasm::InstIndirect::Type::callNormal:
		fPush(0x11);
		break;
	case wasm::InstIndirect::Type::callTail:
		fPush(0x13);
		break;
	default:
		throw wasm::Exception{ L"Unknown wasm::InstIndirect type [", size_t(inst.type), L"] encountered" };
		break;
	}

	/* write the type and table index out */
	binary::WriteUInt(pCode, inst.prototype.index());
	binary::WriteUInt(pCode, inst.table.index());
}
void writer::binary::Sink::addInst(const wasm::InstBranch& inst) {
	/* write the general instruction opcode out */
	switch (inst.type) {
	case wasm::InstBranch::Type::direct:
		fPush(0x0c);
		break;
	case wasm::InstBranch::Type::conditional:
		fPush(0x0d);
		break;
	case wasm::InstBranch::Type::table:
		fPush(0x0e);
		for (size_t i = 0; i < inst.list.size(); ++i)
			binary::WriteUInt(pCode, inst.list.begin()[i].get().index());
		break;
	default:
		throw wasm::Exception{ L"Unknown wasm::InstBranch type [", size_t(inst.type), L"] encountered" };
		break;
	}

	/* write the target index out */
	binary::WriteUInt(pCode, inst.target.index());
}
