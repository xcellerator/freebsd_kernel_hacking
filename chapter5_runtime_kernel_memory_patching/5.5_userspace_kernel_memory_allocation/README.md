# FreeBSD Kernel Hacking

## Chapter 5

### 5.5: Allocating Kernel Memory From Userspace

Armed with the the disassembly of `kmalloc.ko` from [Section 5.4](../5.4_allocating_kernel_memory/), there is a technique that allows us to allocate kernel memory from userland! The only issue is that `kmalloc.ko` contains several relocatable symbols, namely `sys_mkdir`, `M_TEMP`, `malloc`, `memset` and `copyout`. Part of the process will involve finding the memory locations of these symbols with `kvm_nlist()` and calculating the correct offsets as they are referenced by relative jumps/calls.

The technique is as follows:
* Find the memory addresses of the the relocatable symbols and patch their addreses in the `kmalloc` assembly. (The offsets of these addreses are found by simply inspecting the assembly)
* Save `sizeof(malloc)` bytes of the `sys_mkdir` syscall into a buffer for later
* Overwrite `sys_mkdir` with the patched `kmalloc` code
* Call the fake `sys_mkdir`, which actually calls our `kmalloc` code instead
* Restore `sys_mkdir` from the buffer that we saved

To use:
* Make and load the syscall module from [Section 5.4](../5.4_allocating_kernel_memory) with `make; kldload kmalloc.ko`
* Make and run the hook with `make; ./kmalloc_reloaded 16`
* Observe the address of the allocated memory returned to `STDOUT`
* Unload the kernel module with `kldunload kmalloc.ko`
