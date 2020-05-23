# FreeBSD Kernel Hacking

## Chapter 3

### 3.6: Open TCP Port Hiding

Using a similar technique to hiding a process, we can also hide an open tcp port from programs like `netstat`. Instead of the `allproc` list, we target the `V_tcbinfo` list which contains information about all the open connections. In particular, the `V_tcbinfo.listhead` field is full of `inpcb` structs that are the actual entries for each connection. Taking a port number from the arguments passed to the syscall, we can loop through each of these structures with the `CK_LIST_FOREACH` macro, and simply remove it with `CK_LIST_REMOVE` when we get a match.

* An important change in recent versions of FreeBSD that differ from that used in the book is that there isn't a global `V_tcbinfo` object anymore. You have to make a context switch to a virtual network with the `CURVNET_SET()` macro. The unrestricted default network stack is `vnet0`.
* Once we're done, we also have to restore this virtual network once we've released all the mutexes with `CURVNET_RESTORE`.

To use:
* Compile with `make` (you'll need the FreeBSD source headers somewhere like `/usr/src`)
* Load with `kldload ./port_hiding.ko`
* In one tty, run `nc -l -v -n 8001`
* In another, run `nc localhost 8001` (tmux helps with managing these extra ttys!)
* In yet another, run `netstat -anp tcp` and observe that port 8001 is open
* Run `./test.pl`
* Run `netstat -anp tcp` again and observe that port 8001 is now missing, despite the connection still being active in your other ttys!
* Unload with `kldunload port_hiding.ko`
