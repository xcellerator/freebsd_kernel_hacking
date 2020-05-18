#include <sys/types.h>
#include <sys/param.h>
#include <sys/proc.h>
#include <sys/module.h>
#include <sys/sysent.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/malloc.h>

#define AUE_NULL 0

struct kmalloc_args {
	unsigned long size;
	unsigned long *addr;
};

/* System call to allocate kernel memory */
static int kmalloc (struct thread *td, void *syscall_args)
{
	struct kmalloc_args *uap;
	uap = (struct kmalloc_args *)syscall_args;

	int error;

	unsigned long addr;

	/* Allocate kernel memory of size uap->size
	 * M_TEMP = "miscellaneous data buffers" 
	 * M_NOWAIT = malloc will retur NULL if the allocation can't be fulfilled immediately 
	 * If successful, addr will contain the virtual address space that we've been allocated */
	addr = (unsigned long) malloc( (u_long)(uap->size), M_TEMP, M_NOWAIT);
	/* Copy the address of the kernel memory that we've been allocated to userspace
	 * Note that addr is the address of the kernel memory that we've been allocated, but
	 * uap->addr is the variable supplied as an argument by the syscall */
	error = copyout(&addr, uap->addr, sizeof(addr));

	return(error);
}

/* The sysent for the new system call */
static struct sysent kmalloc_sysent = {
	2, 			/* number of arguments */
	kmalloc			/* implementing function */
};

/* The offset in sysent[] where the system call is to be allocated */
static int offset = NO_SYSCALL;		/* first available */

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

SYSCALL_MODULE(kmalloc, &offset, &kmalloc_sysent, load, NULL);
