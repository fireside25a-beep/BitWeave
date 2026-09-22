#!/bin/sh
set -eu
BW=${1:-./bitweave}
D=$(mktemp -d)
trap 'rm -rf "$D"' EXIT
cat > "$D/all.bwx" <<'EOT'
nop
push rax
push r8
pop r8
pop rax
inc rax
dec rbx
not rcx
neg rdx
mov rax, rbx
mov r8, r9
mov r10, 123456
mov rax, [rsp+8]
mov [rsp+16], rax
mov r11, [r12+32]
mov [r13-24], r14
lea rsi, [rip+data]
movzx rdi, [rip+bytev]
movsx rbx, [rip+bytev]
imul rax, rbx
shl rax, 3
shr rbx, 2
sar rcx, 1
add rax, rbx
or rax, rcx
and rax, rdx
sub rax, rsi
xor rax, rdi
cmp rax, r8
test rax, r9
add rax, 1
or rbx, 2
and rcx, 3
sub rdx, 4
xor rsi, 5
cmp rdi, 6
cmovo rax, rbx
cmovno rax, rbx
cmovb rax, rbx
cmovae rax, rbx
cmove rax, rbx
cmovne rax, rbx
cmovbe rax, rbx
cmova rax, rbx
cmovs rax, rbx
cmovns rax, rbx
cmovp rax, rbx
cmovnp rax, rbx
cmovl rax, rbx
cmovge rax, rbx
cmovle rax, rbx
cmovg rax, rbx
jo L0
jno L0
jb L0
jae L0
je L0
jne L0
jbe L0
ja L0
js L0
jns L0
jp L0
jnp L0
jl L0
jge L0
jle L0
jg L0
jmp L0
call L1
L0:
leave
ret
L1:
ret
data:
bits 10010000 10010000 10010000
bytev:
bits 10010000
syscall
EOT
"$BW" build-bits "$D/all.bwx" "$D/a.bits"
"$BW" build-bits "$D/all.bwx" "$D/b.bits"
cmp "$D/a.bits" "$D/b.bits"
[ -s "$D/a.bits" ]
printf 'PASS authoring matrix: all documented forward instruction families encode deterministically\n'
