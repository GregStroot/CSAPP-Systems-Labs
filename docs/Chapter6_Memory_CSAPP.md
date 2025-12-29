# Chapter 6: Memory Hierarchy

## 6.1 Storage technologies

* SRAM each cell is implemented with a 6 transistor circuit that can indefinitely (as long as kept powered) stay in either oof two different voltage configurations
* DRAM (RAM) stores each bit as a charge in a capacitor that must be refreshed every 10-100 milliseconds, which is very long for clock cycles on O(1 nanosecond)
    * Very susceptible to small perturbations (light etc).

* NOTE: We skipped the rest of Secion 6.1 for now. It covers very interesting aspects of Memory design and I/O devices

## 6.2 Locality

* A function such as sumvec that visits each element of a vector sequentially is said to have a *stride-1* reference pattern

* NOTE : This chapter is very similar to DIS CH 11
* TODO : 6.7, 6.8

## 6.4 Cache Memories

* GOAL : Make rigorous the theory we learned in Ch 11 DIS
    * Should enable us to do cache lab

### 6.4.2 Direct-Mapped Cache

* The process that a cache goes through of determining if a request is a miss and then extracting the requested word consists of three steps:
    1. Set selection
    2. Line matching
    3. Word extraction


* Set selection: Extract the *s* set index bits from the middle of the address
    * Selects the *i* index of the set in the cache
* Line matching: Occurs if the valid bit is set *and* the tag matches
* Word extraction: Once we have a hit, we know that *w* is somewhere in the block
    * The block offset bits provide us the offset of the first byte of the desired word (because we aren't often requesting the whole data block [64 bytes])


*  For example, in Intel Core i7 systems, the L1 and L2 caches are **8-way associative**, and the L3 cache is **16-way**
    * Due to the trade off of hit time and miss penalty

## 6.5 Writing Cache-Friendly Code

* The general approach is:
    1. Make the common case go fast
        - Functions spend most of their time in a few loops, so focus on that core functionality
    2. Minimize the number of cache misses in each inner loop

* In general, if a cache has *B* bytes, then a stride-*k* reference pattern results in an average of:
    **min(1, (word size x k)/B)** misses per loop iteration
    * Where $k=1$ minimizes this
    * e.g. (for a sumvec function) suppose we have v block aligned, words are 4 bytes, and cache blocks are 4 words, then we only miss once every 4 loops

* The main takeaway to avoid thrashing and trying to maximize locality as much as possible


## 6.6 Putting It Together: The Impact of Caches on Program Performance

### 6.6.1 The memory mountain

* To summarize our discussion of the memory mountain, the performance of the memory system is not characterized by a single number. Instead, it is a mountain of temporal and spatial locality whose elevations can vary by over an order of magnitude.

### 6.6.2 Rearranging Loops to Increase Spatial Locality

* Six different implementations of matrix multiply (Basic O(n^3) algorithm)
    * 3 equivalence classes:
        1. AB - references A and B in inner most loop
            * Scans row A with stride 1
                - 8 bytes and 32 byte block size => 0.25 misses per loop
            * Scans row B with stride n
                - 1 miss per loop (since n is large)
            * Requires 2 loads (sum is stored in register)
        2. AC - references A and C
            * C with stride n
                - 1 miss per loop
            * A with stride n
                - 1 miss per loop
            * Requires 2 loads and a save
        3. BC - references B and C
            * B with stride 1
                - 0.25 misses per loop
            * C with stride 1
                - 0.25 misses per loop
            * Requires 2 loads and a save

* We see an interesting trade off between AB and BC
    * BC gives lower miss rate, but with an extra operation
    * Figure 6.46 shows that BC is the more performant case
        - **Miss rate, in this case, is a better predictor of performance than the total number of memory accesses**
        - For large values of n, the performance of BC is constant (per inner loop) even though the array is much much alrger than SRAM
            * The prefetching software is smart enough to keep up with stride-1 access pattern!

* Another way to improve performance is by *blocking*, where we organize the data structure into large chunks called *blocks*
    - Then the blocks are loaded into L1 cache, does all reads and write, and discards the chunk, and so on
    - This generally makes the code much much harder to read
    - Here blocking does not help because of the very efficient prefetcher

### 6.6.3 Exploiting Locality in your programs

* In general we recommend:
    1. Focus on inner loops
    2. Maximize spatial locality with stride 1
    3. Maximize temporal locality by using a data object as often as possible once it has been read into memory




--------------------------------
** Problems **
--------------------------------
* Pb 6.17

    * Direct mapped cache:
        * => Need to think about how each setIdx maps

| Data | Address (Decimal) | Cache Set |
| :--- | :--- | :--- |
| `src[0][0]` | 0 | 0 |
| `src[0][1]` | 4 | 0 |
| `src[1][0]` | 8 | 1 |
| `src[1][1]` | 12 | 1 |
| `dst[0][0]` | 16 | 0 |
| `dst[0][1]` | 20 | 0 |
| `dst[1][0]` | 24 | 1 |
| `dst[1][1]` | 28 | 1 |

1. Load `src[0][0]` -> set 0 (and store in register)
    * miss
2. Store `dst[0][0]` -> set 0
    * miss
3. Load `src[0][1]` -> set 0 (and store in register)
    * miss
4. Store `dst[1][0]` -> set 1
    * miss
5. Load `src[1][0]` -> set 1
    * miss
6. Store `dst[0][1]` -> set 0
    * miss
7. Load `src[1][1]` -> set 1
    * Hit!
8. Store `dst[1][1]` -> set 1
    * Miss



| XX    | dst_array |       | src_array |       |   |
| XX    | Col 0     | Col 1 | Col 0     | Col 1 |   |
| Row 0 | m         | m     | m         |  m    |   |
| Row 1 | m         | m     | m         |  h    |   |


* Even though we had spatial locality for src, the conflict of the set indicies caused **thrashing**
    * Leading to even more cache misses!

* What if the cache was 32 data-bytes?
    * Gives us 4 sets (and only 4 cache misses due to no thrashing)
| Data | Address (Decimal) | Cache Set |
| :--- | :--- | :--- |
| `src[0][0]` | 0 | 0 |
| `src[0][1]` | 4 | 0 |
| `src[1][0]` | 8 | 1 |
| `src[1][1]` | 12 | 1 |
| `dst[0][0]` | 16 | 2 |
| `dst[0][1]` | 20 | 2 |
| `dst[1][0]` | 24 | 3 |
| `dst[1][1]` | 28 | 3 |

| XX    | dst_array |       | src_array |       |   |
| XX    | Col 0     | Col 1 | Col 0     | Col 1 |   |
| Row 0 | m         | h     | m         |  h    |   |
| Row 1 | m         | h     | m         |  h    |   |
--------------------------------
--------------------------------
