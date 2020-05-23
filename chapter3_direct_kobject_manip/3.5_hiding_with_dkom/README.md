# FreeBSD Kernel Hacking

## Chapter 3

### 3.5: Hiding a Process with Direct Kernel Object Manipulation (DKOM)

The purpose of this section was to fill in the rest of the gaps in [process_hiding_pidhashtbl.c](../3.4_hiding_a_process_redux/process_hiding_pidhashtbl.c) using all the cleanup functions in [sys/kern/kern_exit.c](https://github.com/freebsd/freebsd/blob/master/sys/kern/kern_exit.c) of the FreeBSD source code, i.e. use the actual source to mimic how FreeBSD cleans up after a process exits. In particulr, if our secret process had a parent, even after employing the previous methods, it would still exist in its parent's child list.

To use:
* Compile with `make` (you'll need the FreeBSD source headers somewhere like `/usr/src`)
* Load with `kldload ./process_hiding.ko`
* In one tty, run `top`
* In another, run `ps` and observe that `top` is in the list
* Run `./test.pl`
* Run `ps` again and observe that `top` is now missing, despite still running!
* Unload with `kldunload process_hiding.ko`
