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

# Section 1.5
* Because of physical laws, larger storage devices are slower than smaller storage devices.

* L1 Cache O(1e4 bytes) -- Nearly same speed as register
* L2 Cache O(1e6 bytes) -- 5 times longer than L1 (still 5-10x faster than main)
* Most modern systems have L3 cache (which is often the unified cache for all cores)

* **Through software design our goal is to perform most memory operations using the fast cache**

# Section 1.6

* The main idea of *memory heirarchy* is that storage at one level serves as a cache for storage at the next lower level

# Section 1.7

* The main purpose of the OS is:
    1. Protect hardware from misuse by runaway apps.
    2. Provide applications with simple and uniform mechanisms for manipulating complicated and, often, wildly different low-level hardware devices


## 1.7.1

* The OS manages concurrently running programs through *context switching* and this is managed by the *kernel*

## 1.7.3

* *Virtual memory* is an abstraction that provides each process with the illusion that it has exclusive use of the main memory.
* Each process has the same uniform view of memory, which is known as its virtual address space
    * In Linux, the **topmost region** of the address space is reserved for code and data in the operating system that is common to all processes.
    * The **lower region** of the address space holds the code and data defined by the user’s process.
* The *virtual address space* is broken up into multiple well-defined areas
    1. Program code and data
    2. Heap
        * Dynamically expands and contracts during execution of a program
    3. Shared Libraries
    4. Stack
        * Compiler uses this to implement function calls (Ch 3)
        * Dynamically expands and contracts during execution of a program
    5. Kernel virtual memory


# 1.9.2

## Thread-level concurrency

*

## Instruction-Level Parallelism

* For example, early microprocessors, such as the 1978-vintage Intel 8086, required multiple (typically 3–10) clock cycles to execute a single instruction.
* More recent processors can sustain execution rates of 2–4 instructions per clock cycle.
* Any given instruction requires much longer from start to finish, perhaps 20 cycles or more, but the processor uses a number of clever tricks to process as many as 100 instructions at a time.
    * This 'concurrency' is enabled through the use of **pipelining**, where the actions required to execute an instruction are partitioned into different steps and the processor hardware is organized as a series of stages, each performing one of these steps.
        * The stages can operate in parallel, working on different parts of different instructions.

## Single Instruction, Multiple-Data (SIMD) Parallelism

* At the lowest level, many modern processors have special hardware that allows a single instruction to cause multiple operations to be performed in parallel, a mode known as single-instruction, multiple-data (SIMD) parallelism.
    * For example, recent generations of Intel and AMD processors have instructions that can add 8 pairs of single-precision floating-point numbers (C data type float) in parallel.
