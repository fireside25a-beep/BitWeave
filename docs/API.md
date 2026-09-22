# BitWeave command API

BitWeave 1.4.0 exposes a forward-authoring command surface.

## Build commands

- `build-bits source.bwx out.bits` — encode new `.bwx` source into canonical textual bits.
- `build-linux-exec source.bwx out.elf [out-full.bits]` — build a Linux x86-64 `ET_EXEC` image.
- `build-linux-pie source.bwx out.pie [out-full.bits]` — build a Linux x86-64 PIE / `ET_DYN` image.
- `build-win64 source.bwx out.exe [out-full.bits]` — build a Windows x86-64 PE32+ image.
- `pack in.bits out.bin` — map every eight canonical textual bits to one byte.
- `canon in.bwa out.bits` — remove comments/annotations from authored bit text and emit canonical bits.

## Verification and metadata

- `verify in.bits built-file` — exact byte-for-byte comparison.
- `sha256 file` — built-in SHA-256.
- `manifest file [target] [kind]` — deterministic release manifest.
- `abi name` — print one of the shipped ABI references.

## Optional AI authoring

- `ai-author-prompt brief.txt` — emits a deterministic prompt asking a local model to write a **new complete `.bwx` program** from the supplied brief.

The model is optional. The resulting `.bwx` still goes through the same deterministic build path as human-authored source.
