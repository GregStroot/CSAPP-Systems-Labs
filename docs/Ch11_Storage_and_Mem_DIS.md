# Chapter 11 Study notes

* Two algorithms with the same asymptotic performance may perform very differently based on their memory access patterns -- these patterns are called *memory locality*
    * Ex: Summing a 2-D array in i-j vs j-i leads to a 5x slower runtime even though they perform the 'same work'
        - Stride 1 vs Stride N
        - Due to C's row-major storage format

## 11.1 Memory Hierarchy

[Memory Hierarchy](https://diveintosystems.org/book/C11-MemHierarchy/_images/MemoryHierarchy.png)
    * Register (1 cycle)
        - On the CPU & Primary
    * Cache (~10 cycles)
        - On the CPU & Primary
        - Broken down into L1-L3
    * Main mem (~100 cycles)
        - Primary
    * Flash disk (~1 M cycles)
    * Traditional Disk (~10 M cycles)

## 11.2 Storage Devices

* **Primary Storage** devices can be accessed *directly* by a program on the CPU
    * CPU's assembly instructions can encode the exact location of the data the instructions should retrieve
        - On IA32 this is accessed via %reg

* CPU instructions can **not** refer directly to secondary storage devices and it must first request the device copy its data into primary storage (e.g. in RAM)


[Primary storage and memory bus architecture](https://diveintosystems.org/book/C11-MemHierarchy/_images/MemoryBus.png)
* To obtain information from the RAM, the CPU must obtain it through calls to the memory bus then the information is put into the register:
```
mov rax, [rbx]       ; 1. Request: Load value at memory address in rbx into register rax
add rax, rcx         ; 2. Work: Add value in rcx to rax (happens inside CPU)
mov [rdx], rax       ; 3. Store: Write the result back to memory at address rdx
```
    * `mov` first looks in L1 (if it's not there, it's a *miss*) then L2 .. all the way to RAM
    * If it hits RAM, it doesn't grab just a number. It grabs a 64-byte **Cache line**
        - This is because the wires are designed for 64-byte 'shipping containers' to reduce the number of calls needed (since you often access sequentially in an array)
    * Going all the way to RAM could cost us 400 clock cycles!

* An important note is that registers are not **NOT** memory. They are **operands**, which the assembly directly references in `mov` or `add` etc. commands


## 11.3 Locality

* Basics of temporal and spatial Locality

## 11.4 Caching

* Caches face several important design questions:
1. Which subsets of a program's memory should the cache hold?
2. When should the cache copy a subset of the programs data from main memory to the cache, or vice versea?
3. How can a system determine whether the programs data is present in the cache?

* To deal with memory and cache management, e.g. what to keep vs not. We can use one of three designs, which are now covered

### 11.4.1 Direct-Mapped Caches

* The cache is broken down into units called *cache lines*
    * Each cache line is independent and stores two pieces of information
        1. Cache data block (Data in blocks ranging from 16 to 64 bytes)
        2. Metadata (Stores information about the contents of the cache line's data block -- e.g. to identify which subset of memory the cache lines data block holds)

[An example of mapping of memory addresses to cache lines](https://diveintosystems.org/book/C11-MemHierarchy/_images/DirectMapping.png)
* In direct-mapped, each address in memory corresponds to exactly one cache line
    * Although each region of memory maps to only one cache line, many memory ranges map to that same cache line and compete for space!
* The key problem with this design is that you could have two pieces of memory needed that compete for the same Cache line -- this is called **Thrashing** and leads to huge slow-downs.

* Use the middle part of the address to reduce competition for the same cache line, effectively letting contiguously stored memory to be stored in nearby cache lines.
    * The memory address is written as:
|Bit Range|Name       |Purpose                                                |
|---------|-----------|-------------------------------------------------------|
|0 - 5    |Byte Offset|Selects 1 of the 64 bytes inside a Cache Line.         |
|6 - 14   |Index      |Selects which Cache Set (the "cubby") to look in.      |
|15 - 47  |Tag        |The "Unique ID" that proves this data is the right one.|


#### Identifying cache contents
* After we take 'middle bits' -> cache line number, we must determine if it holds the requested address
    * The meta-data stores a tag which tells us which of the subset of possible data is stored in the cache line
    *  [Visualisation of check if cache line is proper memory](https://diveintosystems.org/book/C11-MemHierarchy/_images/AddressTag.png)
#### Retrieving Cached Data
* Finally, after verifying we have the proper memory in data, we need to send that data to the CPUs components that need it
    * Typically the cache line (64 bytes) is larger than the requested data (4 bytes), we use the offset portion to identify which bytes of the cache to retrieve -- only electrifying that part of the cache to send it automatically (think of it as the room in the 64 byte cache)
        * So for a 64 byte case 6 bits are reserved in the cache line to be the indicator of what will be sent.

#### Memory Address Division

* Here are the formulas you need to remember for your interviews:

    1. Offset bits = log2(Block Size in bytes)
    2. Index bits = log2(Number of Cache Lines)
    3. Tag bits = Total Address Bits−(Index bits+Offset bits)

* Data Retrieval Flow, the hardware logic executes these three steps in parallel
    1. The index acts like the vertical dimension: It selects the specific row in the SRAM
    2. The tag acts as the check (securicy badge)
    3. The offset, acts as the x-coordiate and tells the multiplexer which specific byte to clip from the 64 byte block


* Example:


    * To put the entire sequence together, follow these steps when tracing the behavior of a cache:

        1. Divide the requested address into three portions, from right (low-order bits) to left (high-order bits): an offset within the cache data block, an index into the appropriate cache line, and a tag to identify which subset of memory the line stores.

        2. Index into the cache using the middle portion of the requested address to find the cache line to which the address maps.

        3. Check the cache line’s valid bit. When invalid, the program can’t use a cache line’s contents (cache miss), regardless of what the tag might be.

        4. Check the cache line’s tag. If the address’s tag matches the cache line’s tag and the line is valid, the cache line’s data block holds the data the program is looking for (cache hit). Otherwise, the cache must load the data from main memory at the identified index (cache miss).

        5. On a hit, use the low-order offset bits of the address to extract the program’s desired data from the stored block. (Not shown in example.)

#### Writing to Cached Data

* Caches allow two ways to store operations
    1. **Write-through cache**, keeping the memory and cache always synchronized
    2. **Write-back cache**, Modifies the value stored in the data block, but does *not* update main memory
        * To manage this, each cache line contains a 'dirty bit', which allows synchronization

* In general, caches further down the hierarchy are more likely to use write-back than write-through
    * Due to higher transfer times as we move down the hierarchy

### 11.4.2 Cache Misses and Associative Designs

* Though we hope to have a good hit rate, there are three scenarios we cannot avoid misses:
    1. **Compulsory misses** -- If a program hasn't accessed the memory location yet
    2. **Capacity misses** -- The program is working with more memory than can be stored in the cache
    3. **Conflict misses** -- The cache design forcing cache misses, e.g. in direct mapped cache
        * The cache design mainly affects the conflict missrate

* The alternative to a direct-mapped cache is a **fully associative** cache where any memory region to occupy any cache location
    * These have the highest lookup and eviction complexity due to every location needing to be considered

* The middle ground is **set associative** caches where every memory region maps to exactly one **cache set**, with each cache set storing multiple cache lines (typically two to eight)
    * These are well suited for general-purpose CPUs

### 11.4.3 Set Associative Caches

* The index still maps to a distinct cache set, but now when performing address lookup, the cache simultaneously checks every line in the set
    * **Question**: How is this now slower than direct-access?
        - The hardware has 8 separate 'Comparators' (a set of logic to check values), so the CPU sends the tag bit to all 8 comparators at once --- Therefore giving a simultaneous matching (instead of a search)
        - This all happens in a few *picoseconds*
        - The eviction is also done by hardware transistor logic and is considered a **zero-instruction** process!
* This works smoothly if we find the data in a cache line, but when it doesn't we must evict another piece of data
    * This is exactly what the LRU Cache (from leetcode) does!
    * Requires keeping extra bits to maintain the ordering of the LRU cache

* 8 way caches are common
    * **Question:** Why not higher or fully associative?
        - The physical energy to charge all the wires (512 for a 32kb L1 cache) would take much longer (10-15 cycles vs 4 cycles) and the trade off isn't worth it for, say market making.
        - Additionally, constantly triggering all 512 Comparator circuits will produce massive amounts of heat, which would likely melt the CPU

## 11.5 Cache Analysis and Valgrind

* Valgrind's *cachegrind* allows us to evaluate cache performance
* A striking analysis of why i-j is so so much faster than j-i for a simple average operator
    * j-i had 1m cache misses and i-j only 20k!

## 11.6 Looking ahead: Caching on multi-core processors

* As we saw in CSAPP, multi-core processors typically have each core with it's own L1 cache then either L2 or L3 as a shared cache between all the cores.
[Example multi-core cache](https://diveintosystems.org/book/C11-MemHierarchy/_images/MulticoreCache.png)
* Since there are multiple private L1 caches, which is advantageous due to strong locality of reference, it means we need to ensure that the cache is coherent across the whole of the memory
    - E.g. one core writing to something that is also cached on a different core.
    - Develop **cache-coherence protocol** to deal with this and ensure no operation is performed on *stale* data

### 11.6.2 The MSI protocol

* Three flags are added to be used by the MSI protocol.
    1. The **M** flag -- which indicates that the block has been modified and the core has written to its copy of the cached value
    2. The **S** flag -- the block is unmodified and can be safely shared (e.g. multiple L1 caches can safely store a copy)
    3. The **I** flag -- the caches block is invalid or contains stale data

* On read access:
    * If the cache is M or S, the cached value and be used
    * If in the I state, the new value needs to be loaded before read!
* On write access:
    * If the block is in M state, write to the cache. No changes needed
    * If in the I or S state, notify other cores that the block is being written to.
        - This requires getting the most recent value (if another has the M flag) or just set it to I if in S

* The cost of write take 30-100 cycles to request ownership from the other cores.

* All of these communications cost time and is known as **Coherency Penalty**, which is why 'multithreading' doesn't always lead to faster performance

### 11.6.3 Implementing Cache Coherency Protocols

* To perform the L1 Communication, there is a bus that is shared by all L1 caches and a *snooping cache controller* that listens on the bus for read/writes to blocks that it caches
