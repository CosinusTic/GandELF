## TODO
- Parse .text section
    - Get parsed sections into a linked list (each symbol with fields address, size, and name)

- Parse opcode manually for each section
```asm
<function_name>:
  0x401000:   mov eax, 1
  0x401003:   ret
```


## Future developments:
- Work on stripped binaries by detecting functions heuristically
- Highlight security-sensitive areas (e.g., writable/executable sections)
- Output annotated disassembly like IDA or Ghidra (e.g., comments, xrefs)

CPU instructions REF: http://ref.x86asm.net/coder64.html#x00
