# BitWeave

<p align="center">
  <a href="assets/hero.png"><img src="assets/hero.gif" alt="BitWeave forward binary authoring" width="100%" /></a>
</p>

**BitWeave is a forward native binary authoring engine.**

You write new software in `.bwx`, BitWeave deterministically encodes it into canonical `0/1` bits, then builds a normal native executable for the selected target.

There is no runtime VM and no translation layer inside the generated program.

## Forward workflow

<p align="center">
  <a href="assets/workflow.png"><img src="assets/workflow.gif" alt="BitWeave forward build workflow" width="100%" /></a>
</p>

```text
.bwx source
    ↓
build-bits
    ↓
canonical .bits
    ↓
native container
    ↓
ELF / PIE / PE32+
    ↓
verify
    ↓
run directly on target
```

BitWeave starts with software authored inside BitWeave and moves forward toward native execution.

## `.bwx` authoring

<p align="center">
  <a href="assets/authoring.png"><img src="assets/authoring.gif" alt="BitWeave authoring and build core" width="100%" /></a>
</p>

`.bwx` is BitWeave's native authoring format.

It supports:

* labels
* calls and jumps
* conditional control flow
* native x86-64 register operations
* arithmetic and logic
* shifts
* comparisons
* RIP relative data references
* embedded data
* literal `bits ...` lines for exact byte patterns

Example:

```text
start:
    mov rax, 42
    ret
```

The program can then be converted into canonical binary source:

```bash
bitweave build-bits program.bwx program.bits
```

## Canonical `.bits`

A `.bits` file contains only:

```text
0
1
whitespace
```

Every eight bits represent exactly one byte.

For example:

```text
10111000 00101010 00000000 00000000 00000000
```

This makes the actual binary representation part of the project instead of hiding it behind hexadecimal output.

The canonical bitstream can be packed directly into bytes using `bitpack` or BitWeave itself.

## Native output

<p align="center">
  <a href="assets/api-targets.png"><img src="assets/api-targets.gif" alt="BitWeave commands ABIs and targets" width="100%" /></a>
</p>

Implemented targets:

* Linux x86-64 `ET_EXEC`
* Linux x86-64 PIE / `ET_DYN`
* Windows x86-64 PE32+

Generated programs run as normal native executables.

## Commands

```text
build-bits <source.bwx> <out.bits>

build-linux-exec <source.bwx> <out.elf> [out-full.bits]

build-linux-pie <source.bwx> <out.pie> [out-full.bits]

build-win64 <source.bwx> <out.exe> [out-full.bits]

pack <in.bits> <out.bin>

canon <in.bwa> <out.bits>

verify <in.bits> <built-file>

sha256 <file>

manifest <file> [target] [kind]

abi <sysv-amd64|linux-x86_64-syscall|win64>

ai-author-prompt <brief.txt>
```

## Example

Build BitWeave:

```bash
cmake -S . -B build
cmake --build build -j
```

Build a native Linux PIE:

```bash
./build/bitweave build-linux-pie examples/hello.bwx hello
```

Run it:

```bash
./hello
```

Build the canonical bits first:

```bash
./build/bitweave build-bits examples/hello.bwx hello.bits
```

Pack them manually:

```bash
./build/bitpack hello.bits hello.bin
```

## Verification and provenance

<p align="center">
  <a href="assets/verification.png"><img src="assets/verification.gif" alt="BitWeave verification and provenance" width="100%" /></a>
</p>

BitWeave keeps the relationship between authored source, canonical bits and native output explicit.

A project can keep together:

```text
.bwx source
canonical .bits
hashes
tests
native target information
```

Verification checks that the packed binary matches the canonical bitstream exactly.

```bash
./build/bitweave verify hello.bits hello
```

Generate a SHA-256 digest:

```bash
./build/bitweave sha256 hello
```

Generate a manifest:

```bash
./build/bitweave manifest hello linux-x86_64 pie
```

## Trust model

BitWeave is influenced by the same class of problems discussed in Ken Thompson's **Reflections on Trusting Trust**.

Readable source alone does not tell you everything about the executable that eventually reaches the CPU.

BitWeave keeps an exact canonical binary representation alongside the authoring source so the path can be inspected directly:

```text
authoring
   ↓
canonical bits
   ↓
exact bytes
   ↓
native executable
```

This does not remove every trust problem in computing.

It makes the binary construction path smaller, more explicit and easier to reproduce.

## Optional AI authoring

AI is optional.

It can be used to help produce a new `.bwx` program from a written brief.

The resulting `.bwx` still goes through the normal BitWeave build path:

```text
brief
  ↓
.bwx
  ↓
.bits
  ↓
native executable
```

There is no AI dependency in generated programs.

## ABIs

BitWeave includes information for:

```text
System V AMD64
Linux x86-64 syscall ABI
Windows x64 ABI
```

Query them with:

```bash
./build/bitweave abi sysv-amd64
./build/bitweave abi linux-x86_64-syscall
./build/bitweave abi win64
```

## Tests

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel

ctest --test-dir build --output-on-failure

tests/test.sh build/bitweave build/bitpack
tests/negative.sh build/bitweave build/bitpack
python3 tests/all_bytes.py build/bitweave build/bitpack
tests/authoring_matrix.sh build/bitweave
```

## Repository layout

```text
src/
    BitWeave and bitpack implementation

canonical/
    canonical .bits representation of the tools

examples/
    BitWeave programs and binary source examples

tests/
    functional, negative and deterministic tests

docs/
    architecture, formats, APIs, ABIs and provenance

assets/
    animated and static project diagrams

.github/workflows/
    automated build and test configuration
```

## Documentation

Detailed documentation is available in:

* [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md)
* [`docs/API.md`](docs/API.md)
* [`docs/ABI.md`](docs/ABI.md)
* [`docs/BINARY_SOURCE.md`](docs/BINARY_SOURCE.md)
* [`docs/PROVENANCE.md`](docs/PROVENANCE.md)
* [`docs/TESTING.md`](docs/TESTING.md)

## License

See [`LICENSE`](LICENSE).
