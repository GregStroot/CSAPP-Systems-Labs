# Chapter 1 Notes
> You are poised for an exciting journey. If you dedicate yourself to learning the
concepts in this book, then you will be on your way to becoming a rare “power pro-
grammer,” enlightened by an understanding of the underlying computer system
and its impact on your application programs.


* STANDARD: Most computer systems represent text characters using the ASCII standard that represents each character with a unique *byte-size* integer value.
* DEF: Files such as hello.c that consist exclusively of ASCII characters are known as *text files*. All other files are known as *binary files*.
*

## 1.2 Stage of Compilation
*Figure 1.3*
1. Preprocessing stage  - inserts code from includes
2. Compilation phase    - Convert to assembly
3. Linking phase        - Linkes includes precompiled routines (e.g. printf.o)

## 1.3 Movating factors

Is a switch statement always more efficient than a sequence of if-else statements?
How much overhead is incurred by a function call?
Is a while loop more efficient than a for loop?
Are pointer references more efficient than array indexes?
Why does our loop run so much faster if we sum into a local variable instead of an argument that is passed by reference?
How can a function run faster when we simply rearrange the parentheses in an arithmetic expression?

## 1.4 Hardward Org.

* DEF: A collection of electrical conduits called *buses* that carry bytes of information back and forth between the components
* DEF: Buses are typically designed to transfer fixed-size chunks of bytes known as *words*.
* The number of bytes in a word (the word size) is a **fundamental system parameter** that varies across systems. **The most frequent today is 8 bytes (64 bits)**

* Physically,main memory consists of a collection of *dynamic random access memory* (DRAM) chips.

### CPU

* DEF: At the core of the CPU is a word-size storage device (or *register*) called the *program counter* (PC).
* A processor *appears* to operator according to a /very simple/ instruction execution model, called the *instruction set architecture*
* The processor works by:
1. Reads instruction from memory pointed by PC
2. Interprets the bits in the instruction
3. Performs simple operator
4. Then updates the PC to  point to the next instruction (which may not be contiguous

* The simple operators are few and revolve around:
1. Main memory
2. Register file, which consists of a collection of word-size registers
3. Arithmetic/logic unit (*ALU*)

Examples of some simple operations the CPU may carry out are:
1. Load -- From main memory into register
2. Store -- Copy from a register to main memory
3. Operator -- Copy contents of two registers to the ALU, perform op and store in register
4. Jump -- extract a word from the instruction and copy to PC

* We can distinguish the processor’s instruction set architecture, describing the effect of each machine-code instruction, from its *microarchitecture*, describing how the processor is actually implemented.

__Section 1.5 Stop__
