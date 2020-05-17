#include <sys/types.h>
#include <sys/param.h>
#include <sys/proc.h>
#include <sys/module.h>
#include <sys/sysent.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/sysproto.h>

/* The system call's arguments. */
/* We have to pack the arguments into a struct first */
struct sc_example_args {
	char *str;
};

/* The system call function. */
static int
sc_example(struct thread *td, void *syscall_args)
{
	/* Declare uap to be an instance of the sc_example_args struct */
	struct sc_example_args *uap;
	/* cast the syscall_args argument to a pointer to sc_example_args struct */
	uap = (struct sc_example_args *) syscall_args;

	/* print the string argument passed to the syscall function */
	/* printf will print to the kernel buffer (read via dmesg) */
	/* uprintf will print to STDOUT */
	printf("%s\n", uap->str);

	return(0);
}

/* The sysent for the new system call. */
/* We have to define the syscall with an entry in a sysent structure */
/* This is defined in sys/sysent.h */
static struct sysent sc_example_sysent = {
	1, 		/* number of arguments */
	sc_example	/* implementing function */
};

/* The offset in sysent[] where the system call is to be allocated. */
/* We have to be assigned a syscall number so that userland can call us */
/* Best practice to use NO_SYSCALL, which claims the first availabe (0-456) */
static int offset = NO_SYSCALL;		/* claim the first available offset */

/* The function called at load/unload. */
/* This is the function that handles declaring the rest of the syscall to the kernel when we're loaded/unloaded */
static int
load(struct module *module, int cmd, void *arg)
{
	int error = 0;
	
	switch(cmd) {
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

/* SYSCALL_MODULE is a wrapper around DECLARE_MODULE from sys/sysent.h. */
/* It automatically sets up the moduledata and syscall_module_data structures for us */
SYSCALL_MODULE(sc_example, &offset, &sc_example_sysent, load, NULL);
