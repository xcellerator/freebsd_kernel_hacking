# FreeBSD Kernel Hacking

## Chapter 2

### Keystroke Logging

This time we hook the `sys_read` syscall in order to log keystrokes. We have to check that the the buffer that `sys_read` has written to is both 1 character long, and came from file descriptor `0`, i.e. `STDIN`. If it passes the checks, we print it out to the kernel buffer. The same technique as the `sys_mkdir` syscall is used to implement the hook by overwriting `sys_read`'s entry in `sysent` on module load/unload.

To use:
* Compile with `make` (you'll need the FreeBSD source headers somewhere like `/usr/src`)
* Load with `kldload ./read_hook.ko`
* Login to a new tty and check `dmesg`
* Unload with `kldunload read_hook.ko`
