# FreeBSD Kernel Hacking

## Chapter 6

### 6.5: Hiding a KLD

In order to hide our KLD, we have to remove it from both the `linker_files` and `modules` lists. These are both just doubly-linked lists and can be modified in the same way as previously - via macros!

However, there are a couple of caveats. The first is that `linker_files` has no dedicated mutex, so we have to use `Giant` instead. This is a "catchall" mutex which protects the whole kernel rather than just a specific portion of it.

The second caveat is that the first `linker_file` structure in `linker_files` is the kernel itself and it's `refs` field is incremented for each `linker_file` that is loaded. This means that we have to decrement this value in order for everything to match.

> NOTE: Seeing as you won't be able to unload this kernel module, there's no point in populating the `MOD_UNLOAD` switch case with anything meaningful. If you're running these in a VM (which you should be!), then snapshots are your friend!

To use:
* Build and load the kernel module with `make; kldload ./incognito-0.3.ko`
* Run `hello` and observe the alternate message
* Check `ls /sbin` and observe that `trojan_hello` does *not* appear in the output
* Check `kldstat` and observe that `incognito` does *not* appear in the output
* You can also try `kldunload incognito-0.3.ko`, but it won't work!

> You will not be able to unload this module without restarting
