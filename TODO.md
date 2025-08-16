## TODO
- Parse opcode manually for each section

```asm
0x00000150 <function_name>:
  0x401000:   mov eax, 1
  0x401003:   ret
```


## Future developments:
- Work on stripped binaries by detecting functions heuristically
- Highlight security-sensitive areas (e.g., writable/executable sections)
- Output annotated disassembly like IDA or Ghidra (e.g., comments, xrefs)

CPU instructions table: http://ref.x86asm.net/coder64.html#x00
CPU instuction encoding explained: https://wiki.osdev.org/X86-64_Instruction_Encoding#Legacy_Prefixes

