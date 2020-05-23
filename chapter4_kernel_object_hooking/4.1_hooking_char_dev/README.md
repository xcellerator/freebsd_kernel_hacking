# FreeBSD Kernel Hacking

## Chapter 4

### 4.1: Hooking a Character Device

Using the `cd_example` character device syscall module from [Section 1.6](../../chapter1_lkm/1.6_character_device_modules/), we can experiment with hooking the driver entry switches in the character device switch table (`open`/`close`/`read`/`write`) to provide extra functionality. The `cdevp_list` list is an system-wide struct that contains all the active character devices. Our hook simply prints a bonus message to the user when we try to read from `/dev/cd_example`.

To use:
* Load `cd_example.ko` from [kernel](../../chapter1_lkm/1.6_character_device_modules/kernel/)
* Compile and run `interface.c` from [userland](../../chapter1_lkm/1.6_character_device_modules/userland/) and observe the normal behaviour
* Load the hook module with `kldload ./cd_exaple_hook.ko`
* Run `interface` once again and observe the bonus message inbetween reading and writing to the character device
* Unload the kernel modules
