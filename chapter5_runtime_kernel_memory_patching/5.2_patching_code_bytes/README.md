# FreeBSD Kernel Hacking

## Chapter 5

### Patching Bytes of Kernel Memory

The syscall module `hello.ko` simply prints "FreeBSD Rocks!\n" 10 times to the kernel buffer and then a new line every time it's called. The `fix_hello.c` program patches the kernel module in memory by patching the `jmp` instruction in the for loop with `nop`s.

First, we have to open a kernel file descriptor with `kvm_openfiles` into the currently running kernel. Then we search through the kernel's memory for `nlist` structures which are just symbol name/memory address pairs. Once we find the `hello` kernel module, we save a copy of the first `SIZE` bytes to a buffer. Now, we can search through this buffer for the byte `0xe9` which is a `jmpq` instruction that appears at the end of the `printf`ing for loop, and save the offset that it appears at. Finally, we can simply write back 5 `nop`s (`0x90`) to the memory address of `hello+offset`.

* Note that `SIZE` is defined as `0x40`. This was chosen because we can see from the disassembly of `hello.ko` that byte `0xe9` appears in the first `0x40` bytes. It would be better if we could calculate the actual size of the `hello` module and just search through the whole thing.

To use:
* Make and load the syscall module with `make; kldload ./hello.ko`
* Run `perl -e 'syscall(210);'` and see the string printed 10 times in `dmesg`
* Make and execute the patch with `make; ./fix_hello`
* Run `perl -e 'syscall(210);' again and check `dmesg` to see that the message only appears once!
* Unload the kernel module with `kldunload hello.ko`
