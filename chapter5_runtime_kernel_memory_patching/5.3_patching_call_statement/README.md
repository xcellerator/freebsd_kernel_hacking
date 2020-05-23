# FreeBSD Kernel Hacking

## Chapter 5

### 5.3: Call Statement Patching

In addition to [Section 5.2](../5.2_patching_code_bytes/), we can also patch the `printf` call to a call to `uprintf`, i.e. instead of printing to the kernel buffer, it will print the `STDOUT`.

To do this, we have to use `kvm_nlist()` to lookup `uprintf` as well as `hello` in kernel memory. Secondly, as shown here, the call instruction [`0xe8`](https://www.felixcloutier.com/x86/call) takes a relative address, so we have to calculate the correct address by substracting the address of the instruction *after* `callq` from the address of `uprintf`. Finally, we can just write this address back in front of the `0xe8` byte.

* Note that `SIZE` is now defined as `0x62` which is the total size of the `hello` syscall module

To use:
* Make and load the syscall module with `make; kldload ./hello.ko` from [Section 5.2](../5.2_patching_code_bytes/)
* Run `perl -e 'syscall(210);'` and see the string printed 10 times in `dmesg`
* Make and execute the patch with `make; ./fix_hello_improved`
* Run `perl -e 'syscall(210);' again and check `dmesg` to see that the message only appears once - and is printed to `STDOUT`!
* Unload the kernel module with `kldunload hello.ko`
