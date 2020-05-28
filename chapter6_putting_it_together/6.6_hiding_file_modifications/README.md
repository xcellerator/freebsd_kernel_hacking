# FreeBSD Kernel Hacking

## Chapter 6

### 6.6: Hiding File Modifications

The last issue with our rootkit is that we will still leave traces when we copy `trojan_hello` to `/sbin/` thanks to access, modification and change time updates.

Access and modification updates are relatively straightforward to patch. We can just use `stat()` to get the current filesystem information, store the results in a `timeval` structure, do what we need to do in `/sbin/` and then finally restore the timestamps with `utimes()`.

Hiding change time is a little more tricky. The method I chose to use deviates heavily from the one used in the book. I spent a lot of time trying to annotate the disassembly [`ufs_itimes_locked`](./ufs_itimes_locked.asm), but ultimately I decided it would be easier to just patch the IF statement so that the check always fails.

At address `0xffffffff80ef75d8` is the following instruction:

|`f6 c1 02`|`test $0x02,%cl`|

which checks the lower 16 bits of `rax` to see if the `IN_CHANGE` flag is set for the current inode (see [`ufs/ufs/inode.h`](https://github.com/freebsd/freebsd/blob/9f6817ff4b760f99399e808d0206b9262ec04bde/sys/ufs/ufs/inode.h#L123) and [`ufs/ufs/ufs_vnops.c`](https://github.com/freebsd/freebsd/blob/3fc1420eac76eb8ddf28d6b0715b2f2fe933f805/sys/ufs/ufs/ufs_vnops.c#L174)). Simply patching `0x02` to `0x00` will cause the change time to no longer be updated.

To use:
* Build and execute the trojan and it's loader with `make; ./trojan_loader`
* Observe the output message to confirm the program executed without errors
* If you didn't already have `trojan_hello` in your `PATH`, then you will now

### BONUS: Hiding from Tripwire HIDS
To prove that this technique really does hide our trojan from a realworld HIDS application, you can test it following the instructions below.

* Install and setup Tripwire from [these](https://forums.freebsd.org/threads/tutorial-intrusion-detection-using-tripwire.56813/) instructions. For simplicity, set it to only check the `/sbin` directory.
* Build both the executables in this directory (`trojan_hello` and `trojan_loader`), but also `incognito-03.ko` in [Section 6.5](../6.5_hiding_a_kld).
* Ensure that `hello` from [Section 6.3](../6.3_execution_redirection) is present in `/sbin`
* Run a Tripwire check with `EDITOR=/usr/local/bin/vim tripwire -m c -I` and update the database if necessary
* Run `hello` (from your PATH) and observe the proper message
* Execute `trojan_loader` and load the rootkit with `kldload incognito-0.3.ko`
* Run `hello` again (from your (PATH) and observe the trojan'd message
* Run a Tripwire check again with `EDITOR=/usr/local/bin/vim tripwire -m c -I` and observe that *no changes were detected!*
