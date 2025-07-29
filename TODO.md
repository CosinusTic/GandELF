## TODO
- Parse .text section, and for each symbol:
    - Show the name
    - Get code bytes from its offset/size.
    - Parse the symbol table to extract function names and offsets.
    - Match .symtab entries with .text addresses and disassemble per symbol.
    - Decode opcodes & disassemble

```asm
<function_name>:
  0x401000:   mov eax, 1
  0x401003:   ret
```


## Future developments:
- Work on stripped binaries by detecting functions heuristically
- Highlight security-sensitive areas (e.g., writable/executable sections)
- Output annotated disassembly like IDA or Ghidra (e.g., comments, xrefs)
