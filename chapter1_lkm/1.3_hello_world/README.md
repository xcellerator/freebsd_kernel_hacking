# FreeBSD Kernel Hacking

## Chapter 1

### Hello, World

This is the prototype for all kernel modules on FreeBSD.

* The `load` function is what gets called when the module is loaded/unloaded from the kernel.
* The `hello_mod` `moduledata_t` struct is an argument passed to `DECLARE_MODULE` that declares the event handler (`load`) for the module
* `DECLARE_MODULE` does what it says on the tin

To use:
* Compile with `make` (you'll need the FreeBSD source headers somewhere like `/usr/src`)
* Load with `kldload ./hello.ko`
* Unload with `kldunload hello.ko` 
