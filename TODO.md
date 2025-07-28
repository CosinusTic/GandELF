## TODO
- Parse .text section, and for each symbol:
    - Show the name
    - Get code bytes from its offset/size.
    - Disassemble

## Future developments:
- Work on stripped binaries by detecting functions heuristically
- Highlight security-sensitive areas (e.g., writable/executable sections)
- Output annotated disassembly like IDA or Ghidra (e.g., comments, xrefs)
