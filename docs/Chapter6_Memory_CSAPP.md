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
