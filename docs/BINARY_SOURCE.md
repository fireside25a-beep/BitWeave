# Binary source formats

## `.bwx`

Human-readable forward authoring source. It supports the documented x86-64 instruction subset, labels, control flow, RIP-relative data references, and literal `bits ...` lines.

## `.bits`

Canonical textual bytes containing only `0`, `1`, and whitespace. Eight bits equal one output byte.

## `.bwa`

Annotated bit text accepted by `canon`. Text following `|` or `#` is ignored while canonicalizing, allowing human notes beside exact bit groups.

The authoritative executable output is the native byte image produced by the selected build command.
