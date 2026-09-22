# Native ABI notes shipped with BitWeave

BitWeave is bit-first, but native code still has to obey the target machine's ABI when it calls an OS or other native code.

The engine exposes deterministic summaries with:

```sh
bitweave abi sysv-amd64
bitweave abi linux-x86_64-syscall
bitweave abi win64
```

## System V AMD64

Integer/pointer arguments: `rdi rsi rdx rcx r8 r9`. Return value: `rax`. Callee-saved: `rbx rbp r12 r13 r14 r15`. Stack alignment is 16 bytes before a call.

## Linux x86-64 syscall ABI

System-call number: `rax`. Arguments: `rdi rsi rdx r10 r8 r9`. Return: `rax`. `rcx` and `r11` are clobbered by `syscall`.

## Win64

First four integer/pointer arguments: `rcx rdx r8 r9`. Return: `rax`. Caller provides 32 bytes of shadow space. Stack alignment is 16 bytes at call boundaries. Nonvolatile integer registers include `rbx rbp rdi rsi rsp r12 r13 r14 r15`.

These summaries are references for the current x86-64 emitters; they are not a claim that BitWeave implements every operating-system API. Literal `.bits` can represent any byte sequence regardless of whether a convenience ABI summary exists.
