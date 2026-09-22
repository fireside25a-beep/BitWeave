#!/bin/sh
set -eu
BW=${1:-./bitweave}
BP=${2:-./bitpack}
D=$(mktemp -d)
trap 'rm -rf "$D"' EXIT
pass=0
ok(){ pass=$((pass+1)); printf 'PASS %s\n' "$1"; }
printf '0100000x\n' > "$D/bad.bits"
if "$BW" pack "$D/bad.bits" "$D/o" >/dev/null 2>&1; then exit 1; fi
ok rejects_non_bit_character
if "$BP" "$D/bad.bits" "$D/o2" >/dev/null 2>&1; then exit 1; fi
ok standalone_rejects_non_bit_character
printf '0101010\n' > "$D/short.bits"
if "$BW" pack "$D/short.bits" "$D/o" >/dev/null 2>&1; then exit 1; fi
ok rejects_incomplete_byte
if "$BP" "$D/short.bits" "$D/o2" >/dev/null 2>&1; then exit 1; fi
ok standalone_rejects_incomplete_byte
printf 'mov nope, 1\n' > "$D/bad.bwx"
if "$BW" build-bits "$D/bad.bwx" "$D/o.bits" >/dev/null 2>&1; then exit 1; fi
ok rejects_unknown_register
printf 'jmp missing\n' > "$D/missing.bwx"
if "$BW" build-bits "$D/missing.bwx" "$D/missing.bits" >/dev/null 2>&1; then exit 1; fi
ok rejects_unresolved_label
printf 'TOTAL_NEGATIVE_PASS %s\n' "$pass"
