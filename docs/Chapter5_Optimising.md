# Optimizing Program Performance

**GOAL**: Basic understanding of compilers. Understand Pipelining and Branch Prediction

* Writing efficient code takes several activities:
    1. Choosing the appropriate algorithms and data structs
    2. Writing code that the compiler can effectively optimize
        - The topic of this chapter!
    3. Writing code that can be parallelised
        - Discussed in Chapter 12

* Even the best compilers can be thwarted by (so-called) *optimization blockers*
    - aspects of the program's behavior that depend strongly on the execution environment

* Programmers must understand how these processors work to be able to tune their programs for maximum speed

* To optimise a program, we must:
    1. Eliminate unnecessary work, making the code perform the task as efficiently as possible
       - Requiring understanding of how the processor works
    2. Exploiting *instruction-level parallelism*

## 5.1 Capabilities and Limitations of Optimizing Compilers

* Invoking gcc with option -O1 or higher (e.g., -O2 or -O3) will cause it to apply more extensive optimizations. These can further improve program performance, but they may expand the program size and they **may make the program more difficult to debug using standard debugging tools.**
    - We will find that we can write C code that, when compiled with option `-O1` can vastly outperform a more naiver version compiled with the highest possible optimisation levels

```c
// twiddle1
void twiddle1(long *xp, long *yp) {
    *xp += *yp;
    *xp += *yp;
}

// twiddle2
void twiddle2(long *xp, long *yp) {
    *xp += 2 * *yp;
}
```

* In this example you would think the compiler could generate twiddle1 to be the same as twiddle 2
    - Saving 2 reads and a write
    - It **cannot** though because it's possible `*xp = *yp`, which would generate a different answer!
        - Due to the compiler not knowing if this is/isn't true, it cannot optimize twiddle1 = twiddle2
        - This is called **memory aliasing**
            * This is a **major** optimization blocker
* Another optimization blocker is due to *function calls*:

```c
[long](long) f();

long func1() {
    /* Calls f() four separate times and sums the results */
    return f() + f() + f() + f();

long func2() {
    /* Calls f() only once and multiplies the result by 4 */
    return 4 * f();
}
```

* It is tempting to try to reduce func1() to func2(), but if f() increments a global counter these will not produce the same results
    * The compiler (`gcc`) will play it save and not make aggressive optimisations like func2() even if the global state isn't modified

## 5.2 Expressing Program Performance

* Define *Cycles per element* (CPE) -- a metric for measuring speed
    - Shows an example where loop-unrolling gives 6 CPE vs 9 CPE without

## 5.3 Program Example

* Basic example of combining elements in an array with operation `OP`
    - `-O1` opt shows 2x speed up

## 5.4 Eliminating Loop Inefficiencies

* Recalling discussion in (Sec 3.6.6) that to translate code containing loops into machine-level programs, the **test condition must be evaluated on every iteration of the loop**
    - Meaning `vec_length(v)` is re-computed every loop!
        * Leads to speed up of 1-3 CPE! (10-30%)
    - Another example is `strlen(s)`, which loops through `s` until it hits a null character!
        * Using this in the test condition leads to O(n^2) complexity!
        * Removing this provides a 5e5 speed-up for a string of 1e6!

* The key being that seemingly trivial pieces of code can lead to hidden asymptotic inefficiencies
    - Which may not come out until production-level tests!

## 5.5 Reducing Procedure Calls

* Here we compare two implementations of combine, one with a function call and one without

```c
/* Implementation with function call in loop */
void combine2(vec_ptr v, data_t *dest) {
    long i;
    long length = vec_length(v); // Code motion: length pulled out of loop
    *dest = IDENT;

    for (i = 0; i < length; i++) {
        data_t val;
        /* The Procedure Call Blocker: */
        get_vec_element(v, i, &val);
        *dest = *dest OP val;
    }
}

```

```c
/* Implementation with direct array access */
void combine3(vec_ptr v, data_t *dest) {
    long i;
    long length = vec_length(v);
    /* Get raw pointer to the start of the data array */
    data_t *data = get_vec_start(v);
    *dest = IDENT;

    for (i = 0; i < length; i++) {
        /* Direct memory access: no function call overhead */
        *dest = *dest OP data[i];
    }
}

```

* We find that there is **no speed up at all**
    - Something else in the inner loop is the bottleneck!
        * This is not to say that `get_vec_element` was not inefficient
    - This transformation is one of the steps that *will* lead to greatly improved performance
    - As will be seen, the key slow-down here is the data-dependency in `*dest`

## 5.6 Eliminating Unneeded Memory References

* In this section we look at the impact of unnecessary memory refs

* The assembly for the inner loop of combine3 is given as:
```gas
/* Inner loop of combine3: data_t = double, OP = * */
/* dest in %rbx, data+i in %rdx, data+length in %rax */

.L17:                               # loop:
    vmovsd (%rbx), %xmm0            # Read current product from dest (MEMORY)
    vmulsd (%rdx), %xmm0, %xmm0     # Multiply by data[i]
    vmovsd %xmm0, (%rbx)            # Store product back at dest (MEMORY)
    addq   $8, %rdx                 # Increment data pointer (i++)
    cmpq   %rax, %rdx               # Compare to data+length
    jne    .L17                     # If !=, goto loop

```
* We have a (unnecessary) load->store cycle we must wait for

* The function may be further optimised as:
```c
/* combine4: Accumulate in a local variable */
void combine4(vec_ptr v, data_t *dest) {
    long i;
    long length = vec_length(v);
    data_t *data = get_vec_start(v);
    data_t acc = IDENT;  /* Local accumulator */

    for (i = 0; i < length; i++) {
        acc = acc OP data[i];
    }

    *dest = acc; /* Store result in memory ONLY ONCE */
}
```
    - As the following assembly code shows, using the accumulator `acc` prevents an unnecessary load and store
    - Goes from 2 reads and one write to just a single read!


* The assembly for the inner loop of combine4 is given as:
```gas
/* Inner loop of combine4: data_t = double, OP = * */
/* acc in %xmm0, data+i in %rdx, data+length in %rax */

.L22:                               # loop:
    vmulsd (%rdx), %xmm0, %xmm0     # Multiply acc by data[i] (REGISTER ONLY)
    addq   $8, %rdx                 # Increment data pointer (i++)
    cmpq   %rax, %rdx               # Compare to data+length
    jne    .L22                     # If !=, goto loop

```

* This leads to a speed up from 2.2-5.7x in terms of CPE!
    - The worst being floating point with `OP = *` going from 11 to 5


| Feature | `combine3` Assembly | `combine4` Assembly |
| --- | --- | --- |
| **Instructions per Loop** | 6 (including 2 memory moves) | 4 (including 0 memory moves) |
| **Data Dependency** | **Memory-to-Register-to-Memory** | **Register-to-Register** |
| **Critical Path** | Must wait for Store → Load cycle | Limited only by the latency of `vmulsd` |
| **Efficiency** | Very Poor (Limited by Cache) | Excellent (Limited by ALU) |

## 5.7 Understanding Modern Processors

* So far, optimisations have been architecture-agnostic, now we must consider the specifics of the *microarchitecture*
    - That is, the underlying system design by which a processor executes instructions
* The actual operation of a modern processor is far different from the view that is obtained by looking at machine level programs
    - In the actual processor, a number of instructions are evaluated simultaneously, a phenomenon known as **instruction-level parallelism**
        * In come designs, there can be 100 or more instruction "in flight" at a time!

* Though the detailed design is beyond the scope, there are two different lower bounds characterizing the maximum performance of a program:
    1. The **latency bound** is encountered when a series of operations must be performed in strict sequence, because the result of one operation is required before the next one can begin
        - Thus (the data dependencies) limiting the usage of instruction-level parallelism
    2. The **throughput bound** characterizes the raw computing capacity of the processors functional units
        - This is the ultimate limit on program performance

### 5.7.1 Overall operation

**Explaination of Figure 5.11**

* We describe *superscalar*, which perform multiple operations on every clock cycle and out of order (!), processors
* The overall design as two parts:
    1. The *instruction control unit* (ICU)
        - Responsible for reading a sequence of instruction from memory and generating, from these, a set of primitive operations to perform on program data
    2. The *execution unit* (EU)
        - Executes the operations from ICU
* The ICU reads instructions from an *isntruction cache*
    - In general, it processes far ahead of the currently executing instruction
    - One problem is when then program hits a *branch*, there are two possible directions the program can go
        * Modern processors employ **branch prediction**, guessing where the branch will be taken and the target address
        * Then using **speculative execution** the processor begins fetching and decoding where the branch was predicted, and even beings executing these operations before it was determined if the branch was correct!!
        * If the branch was incorrect, it resets the state to the branch point and goes in the other direction

* In the intel core i7, it has 8 function units:
    * 4 are capable of performing integer operations
    * 2 are capable of floating-point multiplication

* The *retirement unit* keeps track of ongoing processes and makes sure we obey the sequential semantics of the machine-level program
    - Intermediate steps are stored in the *register file*
    - As an instruction is decoded, information is placed into a FIFO queue
        * Info remains here until it is: *retired* (after being confirmed correct) updates the program register, or *flushed* if the instruction was mispredicted

* To speed-up communication of results from instruction to instruction, much of the information can be exchanged among execution units directly!
    - Think of this like a kitchen:
        1. the ICU throws out work order
        2. the EU does work on those orders
        3. If another unit of the EU needs, say an onion, data-forwarding is throwing the onion to the other chef, rather than putting it in the fridge and having the next chef grab it
            - Avoiding the registers altogether
* **Register renaming** is a data-forwarding technique that:
    * By this mechanism, values can be forwarded directly from one operation to another, rather than being written to and read from the register file, enabling the second operation to begin as soon as the first has completed.
    * Register renaming is overcoming the limitation of the compiler only using ~16 registers when it is not always strictly necessary to use that exact register

* With register renaming, an entire sequence of operations can be performed speculatively, even though the registers are updated only after the processor is certain of the branch outcomes.

### 5.7.2 Functional Unit Performance

* Each operation is characterized by:
    1. *Latency* - the total time required to perform the operation
    2. *Issue time* - the minimum number of clock cycles between two independent ops
    3. *Capacity* - The number of functional units capable of performing the op

* These values are shown for a standard modern processor:

| Operation                   | Latency (Cycles) | Issue Time (Cycles) | Capacity |
| ---                         | ---              | ---                 | ---      |
| **Integer Add**             | 1                | 1                   | 4        |
| **Integer Multiply**        | 3                | 1                   | 1        |
| **Floating-Point Add**      | 3                | 1                   | 1        |
| **Floating-Point Multiply** | 5                | 1                   | 2        |
| **Floating-Point Divide**   | 3–15             | 3–15                | 1        |

* Notice the short issue time, which is due to *pipelining*
    - Floating point add is built of 3 stages and each stage can proceed in close succession rather than waiting for the previous one to finish
    - Functional units with issue times of 1 cycle are said to be *fully pipelined*
    - Notice Divide is not pipelined and *must* finish an operation before starting another

* Creating a unit with short latency or with pipelining requires more hardware, especially for more complex functions such as multiplication and floating-point operations.
    * Since there is only a limited amount of space for these units on the microprocessor chip, CPU designers must carefully balance the number of functional units and their individual performance to achieve optimal overall performance.

### 5.7.3 An Abstract Model of Processor Operation

| Function             | Method                  | **Int +** | **Int *** | **FP +** | **FP *** |
| ---                  | ---                     | ---       | ---       | ---      | ---      |
| **combine4**         | Accumulate in temporary | 1.27      | 3.01      | 3.01     | 5.01     |
| **Latency Bound**    | (Reference Hardware)    | 1.00      | 3.00      | 3.00     | 5.00     |
| **Throughput Bound** | (Theoretical Limit)     | 0.50      | 1.00      | 1.00     | 0.50     |

* So far, we see that our `combine4` function is nearly at our latency bound, except for integer addition
    - It indicates that the performance of these functions is dictated by the latency of the sum or product computation being performed

```gas
/* Inner loop of combine4: data_t = double, OP = * */
/* acc in %xmm0, data+i in %rdx, data+length in %rax */

.L22:                               # loop:
    vmulsd (%rdx), %xmm0, %xmm0     # Multiply acc by data[i] (REGISTER ONLY)
    addq   $8, %rdx                 # Increment data pointer (i++)
    cmpq   %rax, %rdx               # Compare to data+length
    jne    .L22                     # If !=, goto loop

```

* For a code segment forming a loop, we can classify the registers accessed into four categories:
    1. *Read-only*
        - Only read, but not modified (e.g.  `%rax`)
    2. *Write-only*
        - Only write, not in our code
    3. *Local*
        - Updated and used in loop with no dependencies from iteration to iteration (e.g. condition registers used by `cmp` and `jne`)
    4. *Loop*
        - Source and destinations for the loop (e.g. `%rdx` and `%xmm0` in our `combine4` example)

* By looking at the *critical path* for `combine4` we see a true data dependency for the multiply in `%xmm0`, which leads to use waiting 5 cycles for the FP multiply
    - This is true for other operations that have a latency greater than 1 (not integer addition!)
        * Integer addition is limited by the data supply flow, which would require a much more detailed analysis to see why

* **Overview**: It may seem that the latency bound forms a fundamental limit on how fast out combining operation can be performed, but:
    * We will next look at how to restructure operations to enhance instruction-level parallelism!
    * We want to transform the program in such a way that our only limitation becomes the throughput bound, yielding CPEs below or close to 1.00


## 5.8 Loop Unrolling
## 5.9 Enhancing Parallelism
### 5.9.1 Multiple Accumulators
### 5.9.2 Reassociation Transformation
## 5.10 Summary of Results for Optimizing Combining Code
## 5.11 Some Limiting Factors





--------------------------------
** Problems **
--------------------------------
* Pb 5.4
    - Compiled `-O2` performs just as well as `compare4`
    1. How does the role of `%xmm0` differ in O1 vs O2?
        - In O1, xmm0 is a just a buffer for one operation, directly taking rbx. In 02, xmm0 is acting as an accumulator buffer
    2. Will the O2 version implement the combine3, including when there is memory aliasing?
        - Yes
    3. Explain why this preserves the behavior or give a counter example
        - Yes the reason being that we are still always writing back to `rbx`, so when the next add comes it will always be in the most up-to-date version
    - This problem is perplexing being `O2` does just as well as `combine4` even though it has an extra load in every loop! This is because the hardware is performing **store-to-load forwarding** with **out-of-order execution**
        * Basically the `vmovsd` can be executed by throwing the result into a *store buffer* and the CPU doesn't have to wait for it to hit the L1 cache (or wherever) before it proceeds!
        * Thus, allowing the `vmovsd` to happen in the background as the `vmulsd` is being performed (at least for floats which take ~3 cycles; you can see integer isn't quite as fast as `combine4`)
