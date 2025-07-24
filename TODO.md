## TODO
1- Parse ELF: Find .text section and symbol table.
    - ELF symbol header table parsed, must get each symbol table now

2- For each function symbol:

    Show the name (or generate one if stripped).

    Get code bytes from its offset/size.

    Disassemble using Capstone (or manually for extra pain).

3- Print formatted disassembly.
