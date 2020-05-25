# FreeBSD Kernel Hacking

## Chapter 6

### Finding instructions to `NOP`

Examining [`sys/ufs/ufs/ufs_vnops.c`](https://github.com/freebsd/freebsd/blob/3fc1420eac76eb8ddf28d6b0715b2f2fe933f805/sys/ufs/ufs/ufs_vnops.c#L174), we see that there are 3 if statements preceeded by a call to `vfs_timestamp`. The final if statement is the one for the `IN_CHANGE` case, which is what we want to patch.

If we now look at [`ufs_itimes_locked.disas`](./ufs_itimes_locked.disas), at address `0xffffffff80ef754b` we see the call to `vfs_timestamp`, so we know we can ignore any instructions prior to this.

The source code tells us that `DIP_SET` is used 7 times, so we are looking for a repeated instance of 7 blocks of assembly. These instances are prepended by `DIP> `. Fortunately for us, we only have to `nop` out the penultimate 3.

> Note that in the final `DIP>` instruction, we see that it's followed by an `addq` instruction, which matches the source code.

The instructions we have to `nop` out are:

| Address | Instructions | Disassembly |
|-|-|-|
|`0xffffffff80ef7627`|`49 8b 4e 38`|`mov 0x38(%r14),%rcx`|
|`0xffffffff80ef762b`|`89 41 48`|`mov %eax,0x48(%rcx)`|
|`0xffffffff80ef7635`|`49 8b 46 38`|`mov 0x38(%r14),%rax`|
