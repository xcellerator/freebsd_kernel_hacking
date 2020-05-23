# FreeBSD Kernel Hacking

## Chapter 5

### 5.6: Inline Function Hooking

Building on the technique in [Section 5.5](../5.5_userspace_kernel_memory_allocation), we can instead preserve the execution of `sys_mkdir` while also introducing new code - without ever loading a kernel module! We will hook `sys_mkdir` so that every time a new directory is created, the string "Hello, world!\n" will be printed to `STDOUT`.

The technique is as follows:
* Allocate a kernel buffer (see [Section 5.5](../5.5_userspace_kernel_memory_allocation))
  * Find the memory addresses of the the relocatable symbols and patch their addreses in the `kmalloc` assembly. (The offsets of these addreses are found by simply inspecting the assembly)
  * Save `sizeof(malloc)` bytes of the `sys_mkdir` syscall into a buffer for later
  * Overwrite `sys_mkdir` with the patched `kmalloc` code
  * Call the fake `sys_mkdir`, which actually calls our `kmalloc` code instead
  * Restore `sys_mkdir` from the buffer that we saved
* Patch the `hello` function code to contain the correct memory addresses for the "Hello, world!\n" string and `uprintf` call
* Copy the `hello` function into the kernel memory
* Copy `sys_mkdir` (up to the instruction before `call`ing `kern_mkdirat`) into kernel memory
* Copy an unconditional `jmp` back to the call to `kern_mkdirat` into kernel memory
* Copy another unconditional `jmp` to the allocated kernel memory to the start of `sys_mkdir`

The flow of execution will now be as follows:
* `sys_mkdir` is called
* Execution jumps to the allocated kernel memory, where the `hello` function executes
* Immediately after `hello` is the function code for the first part of `sys_mkdir`, which now executes
* Then we encounter an unconditional jump back to the original memory address of `sys_mkdir`, offset to the point where we left off (just prior to the call to `kern_mkdirat`)
* Execution flow contains normally

To use:
* Make and load the syscall module from [Section 5.4](../5.4_allocating_kernel_memory) with `make; kldload kmalloc.ko`
* Make and run the hook with `make; ./mkdir_hook`
* Create a directory with `mkdir lol`, and observe the message printed to `STDOUT`!
* Unload the kernel module with `kldunload kmalloc.ko`
