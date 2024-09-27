#include "text-base.h"

std::u8string_view writer::text::MakeType(wasm::Type type) {
	switch (type) {
	case wasm::Type::i64:
		return u8" i64";
	case wasm::Type::f32:
		return u8" f32";
	case wasm::Type::f64:
		return u8" f64";
	case wasm::Type::refExtern:
		return u8" externref";
	case wasm::Type::refFunction:
		return u8" funcref";
	case wasm::Type::v128:
		return u8" v128";
	case wasm::Type::i32:
	default:
		return u8" i32";
	}
}
std::u8string writer::text::MakeId(std::u8string_view id) {
	if (id.empty())
		return {};
	return str::Build<std::u8string>(u8" $", id);
}
std::u8string writer::text::MakeExport(const wasm::Export& exp) {
	if (!exp.valid())
		return {};
	return str::Build<std::u8string>(u8" (export \"", exp.name, u8"\")");
}
std::u8string writer::text::MakeImport(const wasm::Import& imp) {
	if (!imp.valid())
		return {};
	return str::Build<std::u8string>(u8" (import \"", imp.module, u8"\" \"", imp.name, u8"\")");
}
std::u8string writer::text::MakeLimit(const wasm::Limit& limit) {
	if (limit.maxValid())
		return str::Build<std::u8string>(u8' ', limit.min, u8' ', limit.max);
	return str::Build<std::u8string>(u8' ', limit.min);
}
std::u8string writer::text::MakePrototype(const wasm::Prototype& prototype) {
	if (!prototype.valid())
		return {};
	if (prototype.id().empty())
		return str::Build<std::u8string>(u8" (type ", prototype.index(), u8')');
	return str::Build<std::u8string>(u8" (type $", prototype.id(), u8')');
}
