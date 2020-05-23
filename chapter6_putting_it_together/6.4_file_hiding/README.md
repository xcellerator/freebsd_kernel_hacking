# FreeBSD Kernel Hacking

## Chapter 6

### 6.4: Hiding Files

This extends on [Section 6.3](../6.3_execution_redirection) by also hook the `sys_getdirentries` syscall to hide the `trojan_hello` file in `/sbin/`.

To use:
* Follow the same directions as in [Section 6.3](../6.3_execution_redirection)
* Run `ls /sbin` and observe the `trojan_hello` does *not* exist
  * Running `file /sbin/trojan_hello` or similar will show that the file doesn in fact exist after all...
