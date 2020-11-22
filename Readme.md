Memory Allocator

Virtual memory space contain 5 section:
1. Text section
2. Data section
3. BSS
4. Heap
5. Stack

Data, Bss, Heap collectively called as "data segment".
the end of which is changed by pointer named brk or program break.

It's mean brk point to end of the heap.

We can use sbrk to manipulate via brk.

see man brk for more detail.

Reference
1. https://arjunsreedharan.org/post/148675821737/write-a-simple-memory-allocator
2. https://my.eng.utah.edu/~cs4400/malloc.pdf

Todo:
Add calloc and malloc
Use mmap instead of sbrk
Document the code.


