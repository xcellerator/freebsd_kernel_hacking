# FreeBSD Kernel Hacking

## Chapter 5

### Allocating Kernel Memory

Calling `malloc` from kernel space allows us to allocate kernel memory. This means that we can do much more than just patching bytes.

* An important change in FreeBSD 12 is that allocated kernel memory isn't marked as executable by default anymore. This means that we will page fault when `jmp`ing to an allocated region unless we pass the `M_EXEC` flag to `malloc`.

To use:
* Make and load the syscall module with `make; kldload kmalloc.ko`
* Make and run the userland interface with `make; ./interface`
* Observe the address of the allocated memory returned to `STDOUT`
* Unload the kernel module with `kldunload kmalloc.ko`
