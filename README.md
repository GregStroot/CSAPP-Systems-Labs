# CSAPP: Systems Programming & Optimization Portfolio

**Goal:** Mastering low-level memory management, CPU architecture, and code optimization.
**Reference Text:** *Computer Systems: A Programmer's Perspective (3rd Ed)* by Bryant & O'Hallaron.
**Curriculum Basis:** Adapted from [CMU 15-213 (Spring 2023)](https://www.cs.cmu.edu/afs/cs/academic/class/15213-s23/www/schedule.html).
**Reference Lectures:** Lectures from [CMU 15-213 (Fall 2015)](https://scs.hosted.panopto.com/Panopto/Pages/Sessions/List.aspx#folderID=%22b96d90ae-9871-4fae-91e2-b1627b43e25e%22)

This repository contains my solutions to the core "Systems" laboratories, focusing specifically on **hardware sympathy** and **latency optimization** relevant to High-Performance Computing (HPC) and Quantitative Finance.

## 📅 Progress Tracker & Readings

### Phase 1: Data Representation & Bitwise Logic
*Focus: Bit-level manipulation, Two's Complement, IEEE 754 Floating Point.*

- [ ] **Readings:**
    - [x] CSAPP Ch 1         (A Tour of Computer Systems)
    - [ ] CSAPP Ch 2.1       (Information Storage)
    - [ ] CSAPP Ch 2.2 - 2.3 (Integer Representation & Arithmetic)
    - [ ] CSAPP Ch 2.4       (Floating Point: IEEE 754, Rounding)
- [x] **Lab: C Programming Lab**
    - [x] Queue implementation (Pointers & Structs)
- [ ] **Lab: Data Lab**
    - [ ] Integer Puzzles (Bitwise operators only: `&`, `|`, `~`, `^`, `<<`, `>>`)
    - [ ] Floating Point Puzzles (Bit-level float manipulation)
- [ ] **Key Concepts Mastered:**
    - TODO

### Phase 2: Assembly & The Stack
*Focus: x86-64 Architecture, Registers, Control Flow, and Reverse Engineering.*

- [ ] **Readings:**
    - [ ] CSAPP Ch 3.1 - 3.5 (Machine Code, Data, Instructions, Arithmetic)
    - [ ] CSAPP Ch 3.6       (Control: Loops, Switches, Conditional Moves)
    - [ ] CSAPP Ch 3.7       (Procedures: The Stack, Recursion, Passing Arguments)
    - [ ] CSAPP Ch 3.8 - 3.9 (Array Allocation, Structs, Alignment)
    - [ ] CSAPP Ch 3.10      (Buffer Overflow: Memory safety & Stack smashing)
- [ ] **Lab: Bomb Lab**
    - [ ] Defuse Phases 1-4 (Control flow, Switch statements, Integer math)
    - [ ] Defuse Phases 5-6 (Reverse engineering Pointers, Arrays, Linked Lists in ASM)
- [ ] **Key Concepts Mastered:**
    - TODO

### Phase 3: The Memory Hierarchy (Critical)
*Focus: Cache Locality, Blocking, and Loop Optimization.*

- [ ] **Readings:**
    - [ ] CSAPP Ch 6.1 - 6.3 (Storage Tech, Disk vs RAM, Locality)
    - [ ] CSAPP Ch 6.4 - 6.7 (Cache Memories, Writing Cache-Friendly Code)
    - [ ] CSAPP Ch 5.1 - 5.15 (Optimizing Program Performance: Unrolling, Pipelining)
- [ ] **Lab: Cache Lab**
    - [ ] Part A: Write a Cache Simulator (Handling Sets, Lines, Tags, LRU Eviction)
    - [ ] Part B: Optimize Matrix Transpose (Minimizing Cache Misses via Blocking/Tiling)
- [ ] **Key Concepts Mastered:**
    -   TODO

### Phase 4: Dynamic Memory Management
*Focus: The Heap, Allocators, Fragmentation, and Pointers.*

- [ ] **Readings:**
    - [ ] CSAPP Ch 9.9 (Dynamic Memory Allocation)
    - [ ] CSAPP Ch 9.10 - 9.12 (Garbage Collection concepts)
    - [ ] CSAPP Ch 7           (Linking: Symbols, Static vs Dynamic Libraries)
    - [ ] CSAPP Ch 9.1 - 9.6   (Virtual Memory: Pages, Address Translation)
    - [ ] CSAPP Ch 9.7 - 8.8   (Memory Mapping & Dynamic Memory Alloc.)
- [ ] **Lab: Malloc Lab**
    - [ ] **Checkpoint (Part A):** Implement Implicit Free List Allocator.
    - [ ] **Final (Part B):** Upgrade to Explicit/Segregated Free List.
    - [ ] Optimization: Tuning for Throughput (Ops/sec) and Utilization (Memory footprint).
- [ ] **Key Concepts Mastered:**
    -

---

## 🛠️ Environment & Tools
*   **Compiler:** GCC / Clang (Linux Environment)
*   **Debugger:** GDB
*   **Profiling:** Valgrind, Perf
*   **Build System:** Make

## 📝 Notes on Optimization (Self-Reference)
*(This section to document specific "Aha!" moments useful for interviews)*
*   *Example: *
