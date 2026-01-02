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

* Loop unrolling can improve the performance in two ways:
    1. Reducing the number of ops that do not contribute to the result (e.g. loop indexeing and conditional branching)
    2. It exposes ways that we can transform the code to reduce the number of ops in the critical path

* (k x 1) loop unrolling is when you perform an increment of `k` for your loop idx and perform `k` ops at a time
    - This allows us to get down to the latency bound of 1 for Int + !
        * Because we got rid of the number of overhead ops (first way loop unrolling speeds up)

| Function             | Method       | **Int +** | **Int *** | **FP +** | **FP *** |
| ---                  | ---          | ---       | ---       | ---      | ---      |
| **combine4**         | No unrolling | 1.27      | 3.01      | 3.01     | 5.01     |
| **combine5**         | unrolling    | 1.01      | 3.01      | 3.01     | 5.01     |
|                      | unrolling    | 1.01      | 3.01      | 3.01     | 5.01     |
| **Latency Bound**    |              | 1.00      | 3.00      | 3.00     | 5.00     |
| **Throughput Bound** |              | 0.50      | 1.00      | 1.00     | 0.50     |

* This cannot get us past the latency bound, which we can understand by looking at the assembly:

```gas
.L35:                               # loop:
    vmulsd (%rax,%rdx,8), %xmm0, %xmm0     # 1. Multiply acc by data[i]
    vmulsd 8(%rax,%rdx,8), %xmm0, %xmm0   # 2. Multiply acc by data[i+1]
    addq   $2, %rdx                        # 3. Increment index i by 2
    cmpq   %rdx, %rbp                      # 4. Compare i to limit
    jg     .L35                            # 5. If limit > i, goto loop

```

* `vmulsd` gets translated into top ops: one to load an array element and one to multiply.
* Regardless, the critical path still involves n mul operations in sequence, so (k x 1) unrolling cannot fix that

## 5.9 Enhancing Parallelism

* Our function has hit the limits imposed by the latencies of the ALU, but we can start a new operations every clock cycle (due to being pipelined) and run multiple computations at once
* Currently our code cannot take advantage of this, since we are using a single variable accumulator

### 5.9.1 Multiple Accumulators

* For a combining operation that is associative and commutative, we can improve performance by splitting the set of combining ops into two or more sets

* This may be combined with our previous form of loop unrolling to net us '2 x 2 loop unrolling':
```c
/* 2 x 2 loop unrolling */
void combine6(vec_ptr v, data_t *dest)
{
    long i;
    long length = vec_length(v);
    long limit = length - 1;
    data_t *data = get_vec_start(v);
    data_t acc0 = IDENT;
    data_t acc1 = IDENT;

    /* Combine 2 elements at a time using two independent accumulators */
    for (i = 0; i < limit; i += 2) {
        acc0 = acc0 OP data[i];
        acc1 = acc1 OP data[i+1];
    }

    /* Finish any remaining elements */
    for (; i < length; i++) {
        acc0 = acc0 OP data[i];
    }

    *dest = acc0 OP acc1;
}

```

* This modification pushes us closer to the throughput bound:
| Function             | Method                  | **Int +** | **Int *** | **FP +** | **FP *** |
| ---                  | ---                     | ---       | ---       | ---      | ---      |
| **combine4**         | Accumulate in temporary | 1.27      | 3.01      | 3.01     | 5.01     |
| **combine5**         | unrolling               | 1.01      | 3.01      | 3.01     | 5.01     |
| **combine6**         | unrolling               | 0.81      | 1.51      | 1.51     | 2.51     |
| **Latency Bound**    |                         | 1.00      | 3.00      | 3.00     | 5.00     |
| **Throughput Bound** |                         | 0.50      | 1.00      | 1.00     | 0.50     |

* As expected we see a speed up of around 2x, except for int addition, which still has too much loop overhead
    - With a k=7 and k x k we are able to reach a CPE of 0.54 for Integer addition
    - Floating point mult. reaches 0.51 at k=10!

* In general, a program can achieve the throughput bound for an op only when it can keep the pipeliens filled for all of the functional units capable of performing that op.
    - For an operation with latency *L* and capacity *C*, thie requires an unrolling factor of:
        ` k >= C \dot L`
        * e.g. Floating point mult. has C= 2 and L = 5, thus necessitating k=10

* We must consider if this transformation preserves the functionality
    - For integer data type, the result from `combine4` to `combine6` will be identical and some compilers will do this
    - On the other hand, floating-point multi and addition (on a computer) are **not associated**
        * Most compilers do not attempt such a transformation due to this, but for general systems the gain of 2x outweighs the risk of generating different results for strange data patterns


### 5.9.2 Reassociation Transformation

* We now explore another way to break the sequential dependencies

* By simply reorganizing how the combine operation is done (just the grouping), we can achieve large speed-ups
    * This is known as **'2 x 1a' unrolling**

```c
/* 2 x 1a loop unrolling: Reassociation */
void combine7(vec_ptr v, data_t *dest)
{
    long i;
    long length = vec_length(v);
    long limit = length - 1;
    data_t *data = get_vec_start(v);
    data_t acc = IDENT;

    /* Combine 2 elements at a time with reassociation */
    for (i = 0; i < limit; i += 2) {
        /* Note the parentheses: (data[i] OP data[i+1]) is computed FIRST */
        acc = acc OP (data[i] OP data[i+1]);
    }

    /* Finish any remaining elements */
    for (; i < length; i++) {
        acc = acc OP data[i];
    }

    *dest = acc;
}

```


| Function             | Method                  | **Int +** | **Int *** | **FP +** | **FP *** |
| ---                  | ---                     | ---       | ---       | ---      | ---      |
| **combine4**         | Accumulate in temporary | 1.27      | 3.01      | 3.01     | 5.01     |
| **combine5**         | unrolling               | 1.01      | 3.01      | 3.01     | 5.01     |
| **combine6**         | unrolling               | 0.81      | 1.51      | 1.51     | 2.51     |
| **combine7**         | unrolling               | 1.01      | 1.51      | 1.51     | 2.51     |
| **Latency Bound**    |                         | 1.00      | 3.00      | 3.00     | 5.00     |
| **Throughput Bound** |                         | 0.50      | 1.00      | 1.00     | 0.50     |

* We can see that '2 x 1a' achieves the same speed up as '2 x 2' unrolling for everything except the integer addition case
    - It does this by reducing the length of the critical path. In the '2x1a' route, the CPU can 'pre-compute' the data[i] OP data[i+1] while the previous OP is still being performed on acc. Effectively giving the same speed-up as the 2x2

* Yet again, this technique will not change the results for integer add and multiplication and is often performed for those cases by the compiler
    - BUT, it is not performed for FP's and we (the programmer) must perform that transformation

* Reassociation actually adds another operation versus 'kxk' loop unrolling and this chokes out Int Add, where it takes 1 extra instruction, but saves 5 in the crit. path for FP mult

* ASIDE: There is an extension to SSE (streaming SIMD [single instruction, multiple data] extensions) with most recent versions called **advanced vector extensions** (**AVX**)
    - SIMD involves operating on entire vectors of data with a single instruction!
    - Current AVX registers are 32 bytes long and can allow performing vector ops on 8 (32 bit) values in parallel!
    - This allows us to hit a vector throughput bound of 0.06 for Integer addition or 0.06 for FP mult!

## 5.10 Summary of Results for Optimizing Combining Code

* The following summarizes the result of this chapter  with *scalar* code (not using AVX vector instructions)

| Function             | Method       | **Int +** | **Int *** | **FP +** | **FP *** |
| ---                  | ---          | ---       | ---       | ---      | ---      |
| **combine1**         | Abstract -O1 | 10.12     | 10.12     | 10.17    | 11.14    |
| **combine6**         | unrolling    | 0.81      | 1.51      | 1.51     | 2.51     |
|                      | unrolling    | 0.55      | 1.00      | 1.01     | 0.52     |
| **Latency Bound**    |              | 1.00      | 3.00      | 3.00     | 5.00     |
| **Throughput Bound** |              | 0.50      | 1.00      | 1.00     | 0.50     |

    - Through the use of optimisation we have pushed code to the absolute limits imposed by our functional units performing the operations!
    - Using SIMD instructions, this value can gain an additional 4x or 8x
        * Providing an overall gain of 180x over `-O1`

## 5.11 Some Limiting Factors

* The critical path in a data-flow graph representation indicates a *fundamental lower bound* on the time required to execute a program
* We have also seen that the throughput bound of the functional units also impose a lower bound on the execute time
    - If a program has *N* computations with a processor with *C* functional units and an issue time of *I*
        * Then the program requires `N*I/C` cycles to execute

* Now we consider other limiting factors

### 5.11.1 Register Spilling

* When the program exceeds the number of available registers (by have a high degree of parallelism), the compiler resorts to **spilling**, storing some of the temporary values in memory (typically the run-time stack)
    - We can look at this with loop unrolling:

| Function             | Method    | **Int +** | **Int *** | **FP +** | **FP *** |
| ---                  | ---       | ---       | ---       | ---      | ---      |
| **combine6**         | unrolling | 0.55      | 1.00      | 1.01     | 0.52     |
|                      | unrolling | 0.83      | 1.03      | 1.02     | 0.68     |
| **Throughput Bound** |           | 0.50      | 1.00      | 1.00     | 0.50     |

* We see that the CPEs either don't improve of get worse as we go to `k=20`
    - Modern processors have 16 int/floating point registers

* Thus the compiler spills and for the `k=20` case we must load and store after certain operations!
```
vmovsd 40(%rsp), %xmm0
vmulsd (%rdx), %xmm0, %xmm0
vmovsd %xmm0, 40(%rsp)
```

* **Fortunately**, x86-64 has enough registers that most loops will become throughput limited *before* this occurs


### 5.11.2 Branch Prediction and Misprediction Penalties

* Modern processors *work well ahead* of the currently executing instructions, reading new instructions from memory and decoding them to determine what operations to perform on what operands.

* In general, the prediction of the processors are *very* good and there is minimal overhead for checks that are regular (e.g. checking if we're in the array bounds -- we aren't until the last element)
    * For tests that are completely unpredictable, the branch prediction logic will do *very poorly*

* For **unpredictable cases**, performance can be greatly enhanced if the compiler is able to generate code using __conditional data transfers__ instead of conditional control transfers
    * This cannot be directly controlled, but some ways of expressing conditional behavior can be more directly translated to conditional moves than others
    * We have found the `gcc` is able to generate conditional moves for code written in a more 'functional' style, as opposed to a more 'imperative style'
        - There are no strict rules for these two style, so we illustrate with an example

* Imperative style (2.5-3.5CPE for predictable data and 13.5CPE for random data)
```c
/* Rearrange two vectors so that for each i, b[i] >= a[i] */
void minmax1(long a[], long b[], long n) {
    long i;
    for (i = 0; i < n; i++) {
        if (a[i] > b[i]) {
            long t = a[i];
            a[i] = b[i];
            b[i] = t;
        }
    }
}

```
* Functional style (4.0 CPE regardless of data type):
```c
/* Rearrange two vectors so that for each i, b[i] >= a[i] */
void minmax2(long a[], long b[], long n) {
    long i;
    for (i = 0; i < n; i++) {
        long min = a[i] < b[i] ? a[i] : b[i];
        long max = a[i] < b[i] ? b[i] : a[i];
        a[i] = min;
        b[i] = max;
    }
}

```


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
