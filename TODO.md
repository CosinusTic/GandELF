# TODO

## Corrections
- Correct OPCODES in static mapping 

## Refacto
- cli args handling 

## Future developments:

### Agility
- Work on stripped binaries and detect functions

### Hacking
- Highlight security-sensitive areas (e.g., writable/executable sections)
- print known constants from static maps (e.g. 
    syscalls[50] = {9: "mmap", ...}, 
    errno[50] = {1: "EPERM", ...}, 
    signals[50] = {9: "sigkill", ...})
ex:
```text
mov rax, 9
syscall                     ; syscall mmap
                            ;   addr = 0
                            ;   len  = 0x1000
                            ;   prot = PROT_READ|PROT_WRITE
                            ;   flags= MAP_PRIVATE|MAP_ANONYMOUS
                            ;   fd   = -1, off=0

```
- Get strings in file (read .rodata, .data, .rdata) and get ascii range

### Convenience / debugging
- slice option (--around 0xADDR -n 50): disassemble 50 insns around an address
