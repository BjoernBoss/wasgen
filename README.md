# WebAssembly generation from within C++ to WAT or WASM
![C++](https://img.shields.io/badge/language-c%2B%2B20-blue?style=flat-square)
[![License](https://img.shields.io/badge/license-BSD--3--Clause-brightgreen?style=flat-square)](LICENSE.txt)

Library written in `C++20` to add an interface to generate WAT (WebAssembly Text Format) or WASM (WebAssembly Binary Format). It Supports multi-return, multi-memory, and tail calls for WebAssembly version 1.0.

Currently there is no support for v128 vector operations. Also, no byte-order checks are performed, as the host byte-order is expected to be little-endian, as expected by the WebAssembly standard. Further, the framework does not check for boundary overuns (i.e. creation of more than 2^32 globals and such).

The library does not perform any type checking at construction. It only checks the validity of references and general types used for instructions. But no stack-type verification is performed.

## Using the library
This library consists of multiple cpp files. Simply clone the repository, ensure that `./repos` is on the path (or at least that `<ustring/ustring.h>` can be resolved), ensure that all `cpp` files are included in the compilation, and include `<wasgen/wasm.h>`. The only further requirement is, that the library is compiled using `C++20`.

    $ git clone https://github.com/BjoernBoss/wasgen.git --recursive

The following `cpp` files exist, which need to be included into the compilation:

    wasm-module.cpp
    wasm-sink.cpp
    wasm-target.cpp
    writer/binary/binary-base.cpp
    writer/binary/binary-module.cpp
    writer/binary/binary-sink.cpp
    writer/text/text-base.cpp
    writer/text/text-module.cpp
    writer/text/text-sink.cpp

## Generating a WebAssembly Module

The library lives in the `wasm` namespace. The fundamental idea is to create a `wasm::Module` object, which describes a single module. It takes a `wasm::ModuleInterface` implementation as argument, which is implemented by the `writer::BinaryWriter` and `writer::TextWriter` classes. 

The `writer::BinaryWriter` produces `WASM`, and the `writer::TextWriter` produces a `utf-8` encoded `WAT` string.

Note: When using the library incorrectly, such as defining imports after the first non-imports have been added, a `wasm::Exception` will be thrown.

The following example to produce `WAT`:
```C++
namespace I = wasm::inst;

writer::TextWriter writer{ u8"  " };
wasm::Module mod{ &writer };

wasm::Function imp = mod.function(u8"import_function", {}, {}, wasm::Import{ u8"mod" });

wasm::Memory mem = mod.memory(u8"some_memory", wasm::Limit{ 1, 1 });

wasm::Function fn = mod.function(u8"test_function", { wasm::Type::i32 }, { wasm::Type::i64 }, wasm::Export{});

{
    wasm::Sink sink{ fn };

    sink[I::U32::Const(50)];
    sink[I::Local::Get(sink.parameter(0))];
    sink[I::U32::LessEqual()];
    {
        wasm::IfThen _if{ sink, u8"label", {}, { wasm::Type::i64 } };
        sink[I::U64::Const(1234)];

        _if.otherwise();

        sink[I::Local::Get(sink.parameter(0))];
        sink[I::U32::Const(20)];
        sink[I::U32::Add()];
        sink[I::U32::Expand()];
    }

    sink[I::Call::Direct(imp)];
}

mod.data(mem, wasm::Value::MakeU32(10), { 0x00, 0x01, 0x02, 0x03, 0xff, 0xe0, 0x61, 0x62, 0x63, 0x00 });

mod.close();
const std::u8string& output = writer.output();

str::PrintLn(output);
```

Produces the following example output:

```
(module
  (func $import_function (import "mod" "import_function") (type 0))
  (type (func))
  (memory $some_memory 1 1)
  (type (func (param i32) (result i64)))
  (type (func (result i64)))
  (func $test_function (export "test_function") (type 1)
    i32.const 50
    local.get 0
    i32.le_u
    (if $label (type 2)
      (then
        i64.const 1234
      )
      (else
        local.get 0
        i32.const 20
        i32.add
        i64.extend_i32_u
      )
    )
    call $import_function
  )
  (data (memory $some_memory) (offset i32.const 10) "\00\01\02\03\ff\e0abc\00")
)
```