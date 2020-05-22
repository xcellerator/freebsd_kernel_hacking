# FreeBSD Kernel Hacking

## Chapter 3

### Hiding a Process Redux

It turns out that FreeBSD also maintains a hash table of running processes to aid in lookups. Without also removing the process from this hashtable, it would be possible to examine this hash table and discover our secret process. For example, `ps -p <pid>` would return information about the secret process if we knew the correct PID. We can do this using the `LIST_REMOVE` macro as before.

The alternative to this process is to just use the hash table itself, called `pidhashtbl`, which is what we do in [process_hiding_pidhashtbl.c](./process_hiding_pidhashtbl.c). Fortunately, there is another macro `PIDHASH` for doing these lookups.

To use:
* Compile with `make` (you'll need the FreeBSD source headers somewhere like `/usr/src`)
* Load with `kldload ./process_hiding.ko`
* In one tty, run `top`
* In another, run `ps` and observe that `top` is in the list
* Run `./test.pl`
* Run `ps` again and observe that `top` is now missing, despite still running!
* Unload with `kldunload process_hiding.ko`
