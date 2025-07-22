# Welcome 
This project is a simple x86_64 ELF disassembler written in C intended as a learning
experience in assembly, systems programming, linux binaries, and reverse 
engineering.

The aim of the project is to re-form the assembly (x86) form an ELF binary, like so:

## Aim
Consider the following code:
```C
int add(int a, int b) {
    return a + b;
}

int main() {
    int x = add(2, 3);
    return x;
}
```
... compiled into an ELF binary, the disassembler would form the following asm:
```
0000000000001120 <add>:
    1120:   89 f8                   mov    %edi,%eax
    1122:   01 f0                   add    %esi,%eax
    1124:   c3                      ret    

0000000000001125 <main>:
    1125:   55                      push   %rbp
    1126:   48 89 e5                mov    %rsp,%rbp
    1129:   bf 02 00 00 00          mov    $0x2,%edi
    112e:   be 03 00 00 00          mov    $0x3,%esi
    1133:   e8 e8 ff ff ff          callq  1120 <add>
    1138:   5d                      pop    %rbp
    1139:   c3                      ret    

```

## Project setup
At the root, call ```make```, then execute the **dizzy** program at *./bin* like so ```./bin/dizzy <program_to_disassemble>```

## Authors
Nathan Delmarche
