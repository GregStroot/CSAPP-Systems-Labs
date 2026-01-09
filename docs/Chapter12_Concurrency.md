# Chapter 12: Concurrent Programming

* Modern OS's provide three basic approaches to building concurrent programs:
    1. Processes
    2. I/O Multiplexing
    3. Threads
        * You can think of threads as a hybrid of Processes and I/O Multiplexing
        * Scheduled by the kernel like process flows and sharing the same virtual address space like I/O Multiplexing

* We have skipped notes on 12.1 and 12.2 since they're less relevant to our immediate goal of concurrent programming in C++
## 12.1 Concurrent Programming with Processes
* Each logical flow is a process that is scheduled and maintained by the kernel
    * Processes have **seperate** virtual address space, thus, must communicate via an external explicit *interprodcess communication* mechanism

## 12.2 Concurrent Programming with I/O Multiplexing
* Applications explicitly schedule their own logical flows *in the context of a single process*
    * Logical flow is modeled as a state machine that the main program explicitly transitions from state to state
    * The program is a **single process**, thus, all flows share the same address space

* This is what node.js uses

## 12.3 Concurrent Programming with Threads

* A *thread* is a logical flow that runs in the context of a process
    * Each thread has it's own *thread context* including:
        * Thread ID
        * Stack
        * Stack Pointer
        * Program Counter
        * General-purpose registers
        * Condition codes
    * *All threads* running in a process share the entire virtual address space


### 12.3.1 Thread Execution Model

* The execution model for multiple threads is similar to the execution for multiple processes (Fig 12.12)
    * A process beings life as a single thread called the *main thread*
        * Later, the main thread spawns a *peer thread*
        * From there on, the two threads run concurrently, (not parallely at this point!)
        * Control passes to the peer thread via context switching
            * Either because the main thread execute a slow system call (such as `sleep` or `read` or because it is interrupted by the systems interval timer)
            * The *peer thread* executes for a while then passes it back to the main thread

* It differs from processes because the context switch is *significantly* faster
* Additionally, threads are *not* organized in a rigid parent-child hierarchy

### 12.3.5 Reaping Terminated Threads

* Threads wait for other threads by calling a 'join' function
    * It blocks until thread `tid` terminates, then reaps (cleans-up) any memory by the terminated thread

### 12.3.6 Detaching Threads

* At any point in time, a thread is *joinable* or *detached*
    * A joinable thread can be reaped and killed by other threads
        * Its memory resources (such as stack) are not freed until it is reaped
    * A detached thread cannot be reaped or killed by other threads
        * Its memory resources are freed automatically by the system when it terminates
* By default threads are joinable, hence, to avoid memory leaks, each joinable thread must be reaped or detached

* Join isn't just about memory cleanup. It is also about synchronization!
    * For instance if the main thread is detached and finishes before the others, it will pre-emptively kill them! It doesn't know anything about the other threads or care

* Detached threads are good for 'daemon' tasks -- background jobs that don't return a value and don't have a specific end time that the main program depends on
    * In C++, detached can be dangerous because the thread can outlive the variables they're using and cause a segfault

* C++ 20 has `std::jthread` that automatically calls join to clean-up when out of scope!

## 12.4 Shared Variables in Threaded Programs

* We will work with the below example to show various subtle points about sharing

```cpp
#include <iostream>
#include <vector>
#include <thread>

// Global variable: Shared by all threads
const char **ptr;

void thread_func(int myid) {
    // Static local variable: Exactly one instance shared by all threads
    static int cnt = 0;

    // Accessing shared 'ptr' and shared 'cnt'
    // Note: ++cnt is a race condition!
    printf("[%d]: %s (cnt=%d)\n", myid, ptr[myid], ++cnt);
}

int main() {
    const int N = 2;

    // msgs is a local variable on main's stack
    const char *msgs[N] = {
        "Hello from foo",
        "Hello from bar"
    };

    // ptr is global, but it points to main's stack.
    // This allows peer threads to access main's private stack space.
    ptr = msgs;

    std::vector<std::thread> threads;
    for (int i = 0; i < N; i++) {
        // i is passed by value, becoming a private local variable (myid)
        // on each peer thread's stack.
        threads.emplace_back(thread_func, i);
    }

    // Wait for all threads to finish
    for (auto& t : threads) {
        t.join();
    }

    return 0;
}


```

### 12.4.1 Threads Memory Model

* Each thread has it's own:  a thread ID, stack, stack pointer, program counter, condition codes, and general-purpose register value

* Each thread shares the rest of the process context:
    * Read-only text (code)
    * Read/write data
    * Heap
    * Shared library code/ data areas

* In an operational sense, it is **impossible** for threads to read/write to the registers of another thread, but virtual memory is ALWAYS shared

* The memory model for the seperate thread stacks is not as clean
    * *Usually* the individual stacks are accessed independently by their respective threads
    * That being said, the stacks are **not protected** from the other threads

### 12.4.2 Mapping Variables to Memory

* Variables in a threaded C program are mapped to virtual memory according to their storage class:
    * Global Variables
        - Any variable declared outside of a function
        - At run-time, the r/w area of virtual memory has *exactly* one instance of each global variable
    * Local automatic variables
        - A variable defined inside a function without the `static` attribute
        - At run time, each thread's stack contains its own instance of any local automatic variable
        - True even if multiple threads execute the same thread routine
    * Local static variables
        - A variable defined in a function with a `static` attribute
        - The r/w area of virtual memory contains *exactly* one instance of each local static variable
        - Initialisation (magic static) is thread-safe for C++11 and after

* C++ adds another storage class:
    * thread_local
        - Lives for the duration of the thread and each thread has it's own (but not bound my function stack lifetime)


### 12.4.3 Shared Variables

* A variable *v* is *shared* iff one of its instances is referenced by more than one thread

## 12.5 Synchronizing Threads with Semaphores

* Shared variables are convenient, but they introduce the possibility of nasty *synchronization errors*

```cpp
#include <iostream>
#include <thread>
#include <vector>

/**
 * Figure 12.16: badcnt.c (C++ Version)
 * WARNING: This code is buggy! It illustrates a race condition on a shared variable.
 */

// Global shared variable
// volatile is used here to prevent the compiler from optimizing the loop,
// ensuring every increment attempts to touch memory.
volatile long cnt = 0;

void thread_func(long niters) {
    for (long i = 0; i < niters; i++) {
        // Critical Section: This is not atomic.
        // It involves a Load, an Increment, and a Store.
        cnt++;
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cout << "usage: " << argv[0] << " <niters>" << std::endl;
        return 0;
    }

    long niters = std::atol(argv[1]);

    // Create two threads
    std::thread t1(thread_func, niters);
    std::thread t2(thread_func, niters);

    // Wait for threads to finish
    t1.join();
    t2.join();

    // Check result
    if (cnt != (2 * niters)) {
        std::cout << "BOOM! cnt=" << cnt << std::endl;
    } else {
        std::cout << "OK cnt=" << cnt << std::endl;
    }

    return 0;
}

```

* Figure 12.16 is writing to a global counter with two separate threads
    * Depending on the order this is executed, you could obtain correct or incorrect processing!

* The **crucial point**: *In general, there is no way for you to predict whether the OS will choose a correct ordering for your threads*

### 12.5.1 Progress Graphs

* A *progress graph* models the execution of *n* concurrent threads as a trajectory through an *n*-dimensional Cartesian Space
    * Each axis *k* corresponds to progress on thread *k*
    * Each point (I_1, I_2, ..., I_n) represents the state where thread *k* (*k=1,...,n*) has completed instruction *I_k*

* For thread *i*, the instructions *(L_i, U_i, S_i)* that manipulate `cnt` constitute a *critical section* that should not be interleaved with the crtical section of the other thread
    - That is, we want to ensure that each thread has **mutually exclusive access** to the shared variable while it is executing the instructions in its critical section
        * This is known as **mutual exclusion**

* On the progress graph, the instersection of the two critical sections defines a region of the state known as the *unsafe region*

* In order to guarantee correct execution, we must somehow *synchronize* the threads so that they always have a safe trajectory
    - One (classic) way to do this is via semaphores

### 12.5.2-12.5.3 Semaphores

* Skipped (old C method)

### 12.5.4 Using Semaphores to Schedule Shared Resources

* Here we cover the patterns in CSAPP, but not the methods since we aren't using semaphores
    - Instead we will reference 'C++ Concurrency in Action'


#### Producer-Consumer Problem

* A producer and a consumer thread share a *bounded buffer* with *n* slots
* The producer thread repeatedly produces new *items*
* The consumer thread repeatedly removes items from the buffer then consumes (uses) them
    * There *may be* multiple producers and consumers

* Since we are inserting and removing items involving a shared resource, we must guarantee mutual exclusion
    - However, this alone isn't enough, since we must ensure there is space (for insertion) or it is not empty (for consumption)

* These often come up in real systems (e.g. multi-media systems load and decode a movie)

#### Readers-Writers Problem

* A collection of concurrent threads are acccessing a shared object
    - Some objects can only read the object, while others modify it
    - Threads that modify are *writers*
        * Writers must have exclusive access to the object
    - Threads that only read are *readers*
        * Readers can share with an unlimited number of other readers

* Reader-writers occur frequently in real systems (e.g. airline reservation systems)
* There are several variations of this problem:
    * First readers-writers problem
        - Favors readers and requires no reader is kept waiting unless a write already has permission
    * Second readers-writers problem
        - Favors writers and requires that once a writer is ready to write, it performs the action as quickly as possible

--------------------------------
** Problems **
--------------------------------
* Pb 12.6
### Part A: Variable Instance Analysis

Fill in the table with **Yes** or **No** based on whether the variable instance is accessible/referenced by the specific logical flow.

| Variable Instance | Referenced by Main Thread? | Referenced by Peer Thread 0? | Referenced by Peer Thread 1? |
| ---               | ---                        | ---                          | ---                          |
| **ptr**           | Yes                        | Yes                          | Yes                          |
| **cnt**           | No                         | Yes                          | Yes                          |
| **i.m**           | Yes                        | No                           | No                           |
| **msgs.m**        | Yes                        | Yes                          | Yes                          |
| **myid.p0**       | No                         | Yes                          | No                           |
| **myid.p1**       | No                         | No                           | Yes                          |

---

### Part B: Shared Variables

Based on the mapping above, list which of the variables are considered **shared** (a variable is shared if and only if multiple threads reference at least one of its instances).

* **Shared:** ptr, cnt, msgs.m
    - msgs could have a data race if one thread modified it while the other read
* **Not Shared:** i.m, myid.p0, myid.p1
    - i is not shared because we pass by value, so it is thread-safe

### Contextual Memory Map Reference

* **ptr**: Global variable (Data segment).
* **cnt**: Local static variable (Data segment).
* **i.m**: Local automatic variable on main's stack.
* **msgs.m**: Local automatic variable (array) on main's stack.
* **myid.p0**: Local automatic variable on peer thread 0's stack.
* **myid.p1**: Local automatic variable on peer thread 1's stack.
