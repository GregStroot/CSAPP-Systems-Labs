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


**TODO** Problem 5.4
