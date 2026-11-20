<h1 align="center"> WOODY_WOODPACKER </h1>

<p align="center">
a little packer that pecks your ELF, hides the code inside,<br>
and teaches it to un-hide itself right before it runs c:
</p>

---

### c: what it is

woody_woodpacker is a tiny **ELF64 packer** for Linux (x86-64).

you feed it a binary, it spits out a new one called `woody` that:

- is **encrypted** on disk — the real code is scrambled, unreadable to a curious `objdump`
- **decrypts itself in memory** at launch, at the very last second before running
- prints `....WOODY....` to announce it woke up
- then behaves **exactly** like the original — same output, same return code, no drama

basically the program carries its own key, unlocks itself on the fly, and runs like nothing happened. antivirus-evasion 101, built from scratch to actually understand how real packers tick instead of just reading about them.

### >;] how it works under the shell

the packer (in **C**):

- parses the ELF header + program headers by hand — no libelf, we're not cowards
- hunts down the executable `PT_LOAD` segment (the actual code)
- encrypts it with **RC4** and a **128-bit key** pulled fresh from `/dev/urandom`
- glues a handwritten **assembly stub** onto the end of the file
- recycles a useless `PT_NOTE` segment into a brand new `PT_LOAD` pointing at the stub (the classic Silvio trick — no need to grow the header table)
- hijacks the entry point so the stub runs first

the stub (in **NASM**, fully position-independent):

- prints the `....WOODY....` banner through a raw `write` syscall
- flips the code pages writable with `mprotect` (W^X says hi)
- re-runs RC4 with the embedded key to decrypt the code straight in RAM
- restores every register and **jumps back to the original entry point**

since the target is a **PIE**, the stub never hardcodes an address — it locates itself at runtime (`lea [rel ...]`, RIP-relative) and reaches everything else by fixed offsets. ASLR can drop it anywhere in memory and it still lands on its feet.

### >;] current stack

<p align="left">
<img src="https://img.shields.io/badge/C-00599C?style=for-the-badge&logo=c&logoColor=white" />
<img src="https://img.shields.io/badge/Assembly-525252?style=for-the-badge&logo=assemblyscript&logoColor=white" />
<img src="https://img.shields.io/badge/NASM-ED1C24?style=for-the-badge&logo=gnu&logoColor=white" />
<img src="https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black" />
<img src="https://img.shields.io/badge/GNU%20Make-A42E2B?style=for-the-badge&logo=gnu&logoColor=white" />
<img src="https://img.shields.io/badge/GDB-4EAA25?style=for-the-badge&logo=gnu&logoColor=white" />
</p>

### build & run

```sh
make
./woody_woodpacker <elf64_binary>
# -> key_value: 711ff0922a6a6e7846a9cbeb4cb51942
# -> drops a fresh ./woody next to you

./woody
# -> ....WOODY....
# -> hello world
```

needs `cc`, `nasm`, `xxd` and a 64-bit Linux. zero third-party libraries — the stub talks to the kernel directly.

### what's under the hood (the nerdy bits)

- ELF64 parsing — segments vs sections, PIE / ASLR awareness
- RC4 stream cipher (KSA + PRGA) reimplemented from scratch, random key
- handwritten PIC x86-64 stub, syscalls only, no libc, no relocations
- `PT_NOTE` → `PT_LOAD` segment injection
- runtime self-decryption: `mprotect` + register-preserving jump to the real entry
- valgrind-clean (no leaks, no uninit reads) and crash-proof on garbage input — the error paths got fuzzed with empty, truncated, non-ELF and 32-bit files

### notes / honest disclaimers

- the key lives **inside** `woody`, so this is **not** "secure" — a packer buys obfuscation, not secrecy. crack it open and the key is right there. that's the whole point of the exercise, not a bug c:
- 64-bit ELF only for now (32-bit, PE and Mach-O = future pecking)

---

<p align="center">
<i>a woodpecker with a hedgehog's patience: taps the same spot until the shell cracks c:</i>
</p>
