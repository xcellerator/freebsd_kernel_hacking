# FreeBSD Kernel Hacking

## Chapter 3

### Hiding a Process

Using the `allproc` kernel object (a doubly-linked list), we can iterate through every running process on the system looking for a process with a specific name (which is passed as an argument to the syscall). When we find it, we can remove it from the the `allproc` list, hiding it from the system (it won't show up in `ps` or `top`). This doesn't affect the processes threads or execution in any way.

When making changes to kernel objects, we have to use mutexes to make sure that we are the only thread that can do so to prevent clashes and corruption. This is handled by the `sx_xlock`/`sx_xunlock` functions and the `PROC_LOCK`/`PROC_UNLOCK` macros.

To use:
* Compile with `make` (you'll need the FreeBSD source headers somewhere like `/usr/src`)
* Load with `kldload ./process_hiding.ko`
* In one tty, run `top`
* In another, run `ps` and observe that `top` is in the list
* Run `perl -e '$p="top";syscall(210,$p);'
* Run `ps` again and observe that `top` is now missing, despite still running!
* Unload with `kldunload process_hiding.ko`
