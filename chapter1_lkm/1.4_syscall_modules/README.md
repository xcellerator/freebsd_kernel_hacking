# FreeBSD Kernel Hacking

## Chapter 1

### 1.4: System Calls

Implement a new system call that just prings the argument passed to it to the kernel buffer.

To use:
* Compile the syscall with `make` (you'll need the FreeBSD source headers somewhere like `/usr/src`)
* Load with `kldload ./sc_example.ko`
* Pass a string to either the C program under `userland` or `test.pl`
* Check the kernel buffer with `dmesg` to see that your string got printed
* Unload with `kldunload sc_example.ko`
