# Architecture

BitWeave is intentionally split into a small authoring/build tool and ordinary native output.

```text
.bwx authoring source
        ↓
two-pass layout + native encoding
        ↓
canonical .bits
        ↓
container writer
        ↓
ELF64 / PIE / PE32+
        ↓
verification + manifest
        ↓
runnable native program
```

## Two-pass build

Pass 1 determines label locations and final layout. Pass 2 emits the exact native instruction bytes and fails when a referenced label cannot be resolved.

## Literal bits

A `.bwx` file may contain `bits ...` lines for exact byte data. This keeps a direct path to literal machine bytes without requiring hexadecimal notation.

## Generated runtime

The generated program contains native code and its native executable container. It does not include BitWeave, an interpreter, or a VM.

## Native container writers

The current engine writes Linux x86-64 `ET_EXEC`, Linux x86-64 PIE / `ET_DYN`, and Windows x86-64 PE32+ containers.
