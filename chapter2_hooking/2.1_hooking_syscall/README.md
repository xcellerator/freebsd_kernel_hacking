# FreeBSD Kernel Hacking

## Chapter 2

### 2.1: Syscall Hooking

Write a customized `sys_mkdir` syscall that outputs some extra information and overwrite the real `sys_mkdir`'s entry in the `sysent` structure with the syscall number of our new function. When we unload the module, we have to also write the original syscall number back.

To use:
* Compile with `make` (you'll need the FreeBSD source headers somewhere like `/usr/src`)
* Load with `kldload ./mkdir_hook.ko`
* Create a new folder, and check `dmesg` for the extra output
* Unload with `kldunload mkdir_hook.ko`
