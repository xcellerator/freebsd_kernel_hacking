# FreeBSD Kernel Hacking

## Chapter 6

### 6.3: Execution Redirection

We can hook the `SYS_execve` syscall to run a different program to the one asked for. The good point is that we can avoid HIDS detections because we don't make any changes to files on disk, but the trojan executable will still be on the filesystem - which we will mediate in the next section.

To use:
* Build the two sample programs in `/sample` with `make`
* Symlink them both to `/sbin/`
  * `ln -sf sample/hello /sbin/hello`
  * `ln -sf sample/trojan_hello /sbin/trojan_hello`
* Execute `hello` from your `PATH` and observe the output
* Build and load the kernel module with `make; kldload incognito-0.1.ko`
* Exectute `hello` from `PATH` again and observe the different output!
* Unload the kernel module with `kldunload incognito-0.1.ko`
