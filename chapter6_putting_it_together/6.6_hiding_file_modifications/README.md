# FreeBSD Kernel Hacking

## Chapter 6

### 6.6: Hiding File Modifications *(IN PROGRESS)*

The last issue with our rootkit is that we will still leave traces when we copy `trojan_hello` to `/sbin/` thanks to access, modification and change time updates.

Access and modification updates are relatively straightforward to patch. We can just use `stat()` to get the current filesystem information, store the results in a `timeval` structure, do what we need to do in `/sbin/` and then finally restore the timestamps with `utimes()`.

However, change time is more complicated - we have to patch parts of the kernel memory with `nop`s and then restore it. [`ufs_itimes_locked.asm`](./ufs_itimes_locked.asm) shows the `objdump` output of the function `ufs_itimes_locked` which is defined in [`sys/ufs/ufs/ufs_vnops.c`](https://github.com/freebsd/freebsd/blob/3fc1420eac76eb8ddf28d6b0715b2f2fe933f805/sys/ufs/ufs/ufs_vnops.c#L174) on FreeBSD 12. In this function, we can see that the macro `DIP_SET` is used 3 times in the final if statement. It is these 3 lines that we need to `nop` out.

The method used to find the correct instructions in [`ufs_itimes_locked.asm`](./ufs_itimes_locked.asm) to `nop` out is [here](./NOPPING.md).

To use:
* Build and execute the trojan and it's loader with `make; ./trojan_loader`
* Observe the output message to confirm the program executed without errors
* If you didn't already have `trojan_hello` in your `PATH`, then you will now
