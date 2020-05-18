#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/module.h>

int main (int argc, char *argv[])
{
	int syscall_num;
	struct module_stat stat;

	unsigned long addr;
	if (argc != 2)
	{
		printf("Usage: %s <size>\n", argv[0]);
		exit(0);
	}

	stat.version = sizeof(stat);
	modstat(modfind("sys/kmalloc"), &stat);
	syscall_num = stat.data.intval;

	syscall(syscall_num, (unsigned long) atoi(argv[1]), &addr);
	printf("Address of allocated kernel memory: 0x%x\n", addr);

	exit(0);
}
