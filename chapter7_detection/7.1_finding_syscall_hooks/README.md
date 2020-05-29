# FreeBSD Kernel Hacking

## Chapter 7

### 7.1: Finding Syscall Hooks

Explanation

To use:
* Build and load the `mkdir` hook from [Section 2.1](../../chapter2_hooking/2.1_hooking_syscall/).
* Create a directory with any name, and observe the output in the kernel buffer with `dmesg`
* Build `checkcall` with `make`
* Run `checkcall sys_mkdir 136 fix` (syscall numbers can be found in [sys/sys/syscall.h](https://github.com/freebsd/freebsd/blob/master/sys/sys/syscall.h)
* Create another directory and observe that a message is *not* printed to the kernel buffer a second time
* Unload the kernel module with `kldunload mkdir_hook.ko`

> NOTE: `checkcall` only corrects the address that `sysent[CALL_NO].sy_call` points to - the kernel module itself will remain in memory until it is manually unloaded.
