#pragma once

#include <ustring/ustring.h>
#include <algorithm>
#include <vector>
#include <string>

#include "../wasm/wasm.h"
#include "../util/logging.h"

namespace writer::text {
	class Module;
	class Sink;

	/* convert common types to either the null-string (if empty) or to a string of a leading space, followed by the value */
	std::u8string_view MakeType(wasm::Type type);
	std::u8string MakeId(std::u8string_view id);
	std::u8string MakeExport(const wasm::Export& exp);
	std::u8string MakeImport(const wasm::Import& imp);
	std::u8string MakeLimit(const wasm::Limit& limit);
	std::u8string MakePrototype(const wasm::Prototype& prototype);

	/* convert the operand to a string (without leading space) */
	std::u8string_view MakeOperand(wasm::OpType operand);
}
