#include <sys/types.h>
#include <sys/param.h>
#include <sys/proc.h>
#include <sys/module.h>
#include <sys/sysent.h>
#include <sys/kernel.h>
#include <sys/systm.h>

#define AUE_NULL 0

/* The system call function */
static int hello(struct thread *td, void *syscall_args)
{
	int i;
	/* Annoying output that we will patch out */
	for (i = 0 ; i < 10 ; i++)
		printf("FreeBSD Rocks!\n");
	printf("\n");

	return(0);
}

/* The sysent for the new system call */
static struct sysent hello_sysent = {
	0,		/* number of arguments */
	hello		/* implementing function */
};

/* The offset in sysent[] where the system call is to be allocated */
static int offset = NO_SYSCALL;		// First available

/* The function called at load/unload */
static int load (struct module *module, int cmd, void *arg)
{
	int error = 0;

	switch(cmd)
	{
		case MOD_LOAD:
			uprintf("System call loaded at offset %d.\n", offset);
			break;

		case MOD_UNLOAD:
			uprintf("System call unloaded from offset %d.\n", offset);
			break;

		default:
			error = EOPNOTSUPP;
			break;
	}

	return(error);
}

SYSCALL_MODULE(hello, &offset, &hello_sysent, load, NULL);
