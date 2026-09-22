#!/bin/sh
set -eu
BW=${1:-./bitweave}
BP=${2:-./bitpack}
D=$(mktemp -d)
trap 'rm -rf "$D"' EXIT
pass=0
ok(){ pass=$((pass+1)); printf 'PASS %s\n' "$1"; }
run_exit(){ f=$1; want=$2; rc=0; "$f" >/dev/null 2>&1 || rc=$?; [ "$rc" -eq "$want" ]; }

[ "$($BW --version)" = "1.4.0" ]; ok version
printf abc > "$D/abc"; [ "$($BW sha256 "$D/abc")" = "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad" ]; ok sha256
printf '01000001 01000010\n' > "$D/a.bits"; $BW pack "$D/a.bits" "$D/a.bin"; [ "$(cat "$D/a.bin")" = "AB" ]; ok pack
printf '01000001 | A\n01000010 # B\n' > "$D/a.bwa"; $BW canon "$D/a.bwa" "$D/c.bits"; $BW pack "$D/c.bits" "$D/c.bin"; cmp "$D/a.bin" "$D/c.bin"; ok annotated_canon

cat > "$D/exit42.bwx" <<'EOT'
mov rax, 60
mov rdi, 42
syscall
EOT
$BW build-bits "$D/exit42.bwx" "$D/exit42.bits"; ok build_bits
$BW build-linux-exec "$D/exit42.bwx" "$D/exit42.exec" "$D/exit42.exec.bits"; run_exit "$D/exit42.exec" 42; ok build_linux_exec
$BW build-linux-pie "$D/exit42.bwx" "$D/exit42.pie" "$D/exit42.pie.bits"; run_exit "$D/exit42.pie" 42; ok build_linux_pie
$BP "$D/exit42.pie.bits" "$D/exit42.copy"; cmp "$D/exit42.pie" "$D/exit42.copy"; ok standalone_bitpack
$BW verify "$D/exit42.pie.bits" "$D/exit42.pie" >/dev/null; ok full_image_verify

cat > "$D/branch.bwx" <<'EOT'
mov rax, 5
cmp rax, 5
je yes
mov rdi, 1
jmp done
yes:
mov rdi, 7
done:
mov rax, 60
syscall
EOT
$BW build-linux-pie "$D/branch.bwx" "$D/branch"; run_exit "$D/branch" 7; ok labels_jcc_jmp

cat > "$D/call.bwx" <<'EOT'
call fn
mov rdi, rax
mov rax, 60
syscall
fn:
mov rax, 9
ret
EOT
$BW build-linux-pie "$D/call.bwx" "$D/call"; run_exit "$D/call" 9; ok call_ret

cat > "$D/memory.bwx" <<'EOT'
mov rax, 42
push rax
mov rbx, [rsp]
pop rcx
cmp rbx, 42
jne fail
mov rdi, 11
jmp out
fail:
mov rdi, 2
out:
mov rax, 60
syscall
EOT
$BW build-linux-pie "$D/memory.bwx" "$D/memory"; run_exit "$D/memory" 11; ok stack_memory

cat > "$D/math.bwx" <<'EOT'
mov rax, 6
mov rbx, 7
imul rax, rbx
shl rax, 1
shr rax, 1
cmp rax, 42
jne fail
mov rdi, 13
jmp out
fail:
mov rdi, 3
out:
mov rax, 60
syscall
EOT
$BW build-linux-pie "$D/math.bwx" "$D/math"; run_exit "$D/math" 13; ok arithmetic_shift_imul

cat > "$D/cmov.bwx" <<'EOT'
mov rax, 1
mov rbx, 17
cmp rax, 1
cmove rax, rbx
mov rdi, rax
mov rax, 60
syscall
EOT
$BW build-linux-pie "$D/cmov.bwx" "$D/cmov"; run_exit "$D/cmov" 17; ok cmov

cat > "$D/rip.bwx" <<'EOT'
lea rsi, [rip+message]
movzx rdi, [rip+value]
mov rax, 60
syscall
message:
bits 01001111 01001011 00001010
value:
bits 00010111
EOT
$BW build-linux-pie "$D/rip.bwx" "$D/rip"; run_exit "$D/rip" 23; ok rip_relative_data

cat > "$D/hello.bwx" <<'EOT'
mov rax, 1
mov rdi, 1
lea rsi, [rip+message]
mov rdx, 13
syscall
mov rax, 60
xor rdi, rdi
syscall
message:
bits 01001000 01100101 01101100 01101100 01101111 00101100 00100000 01100010 01101001 01110100 01110011 00100001 00001010
EOT
$BW build-linux-pie "$D/hello.bwx" "$D/hello"; [ "$("$D/hello")" = 'Hello, bits!' ]; ok hello_native_app

cat > "$D/win.bwx" <<'EOT'
xor rax, rax
ret
EOT
$BW build-win64 "$D/win.bwx" "$D/win.exe" "$D/win.exe.bits"; file "$D/win.exe" | grep -Eq 'PE32\+ executable.*x86-64|PE32\+ executable.*Intel'; ok win64_container

$BW manifest "$D/exit42.pie" linux-x86_64 elf64-pie > "$D/m1"; $BW manifest "$D/exit42.pie" linux-x86_64 elf64-pie > "$D/m2"; cmp "$D/m1" "$D/m2"; ok deterministic_manifest
$BW abi sysv-amd64 | grep -q 'rdi rsi rdx rcx r8 r9'; $BW abi linux-x86_64-syscall | grep -q 'r10'; $BW abi win64 | grep -q 'shadow-space 32'; ok abi_tables
printf 'Create a native program that exits with status 5.\n' > "$D/brief.txt"; $BW ai-author-prompt "$D/brief.txt" | grep -q 'BITWEAVE-AI-AUTHOR-1'; ok ai_author_protocol

printf 'TOTAL_PASS %s\n' "$pass"
