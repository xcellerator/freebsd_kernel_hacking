#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/module.h>

#define MODULE_NAME "sys/sc_example"

int main (int argc, char *argv[] )
{
	int syscall_num;
	struct module_stat stat;

	if (argc != 2)
	{
		printf("Usage:\n%s <string>\n", argv[0]);
		exit(0);
	}

	/* Determine sc_example's offset value. */
	stat.version = sizeof(stat);
	/* modfind returns the modid of a kernel module based on its module name */
	/* modstat returns the status of a kernel module based on its modid */
	/* it is returned in a module_stat struct, defind in sys/module.h */
	modstat(modfind(MODULE_NAME), &stat);
	/* the syscall number is store within a struct within a struct */
	syscall_num = stat.data.intval;

	/* Call sc_example */
	return( syscall(syscall_num, argv[1]) );
}
