# FreeBSD Kernel Hacking

## Chapter 1

### 1.6: Character Device

Implement a character device that copies a string from userland into a buffer in kernel space.

* `cd_example_cdevsw` is a "character device switch table" that controls which functions are called when we try to open/close/read/write to/from the new character device.
* The functions that handle this, work very similarly to `strcpy`
* `copyinstr` copies a string *IN* to the kernel from userspace
* `copystr` copies a string *OUT* from the kernel back to userspace

To use:
* Compile the syscall with `make` (you'll need the FreeBSD source headers somewhere like `/usr/src`)
* Load with `kldload ./cd_example.ko`
* Compile the userland program `interface` with `make`
* Feed the syscall a string with `sudo ./interface Hello\ world`
* Unload with `kldunload cd_example.ko`
