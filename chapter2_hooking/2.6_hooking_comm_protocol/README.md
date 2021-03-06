# FreeBSD Kernel Hacking

## Chapter 2

### 2.6: Communication Protocol Hooking

We hook FreeBSD's ICMP packet handler in the network protocol swich table `inetsw`. The hook examines each ICMP packet for the trigger payload "Shiny.". If it finds it, it prints a message to the kernel buffer.

This idea is improved in [icmp_input_hook_extra.c](./icmp_input_hook_extra.c) by first checking the data field in the IP header is long enough and that the data within the packet is actually accessible.

To use:
* Compile with `make` (you'll need the FreeBSD source headers somewhere like `/usr/src`)
* Load with `kldload ./icmp_input_hook.ko`
* Compile [nemesis](https://github.com/troglobit/nemesis) from source for FreeBSD
* Run `./test.sh` and check `dmesg`
* Unload with `kldunload icmp_input_hook.ko`
