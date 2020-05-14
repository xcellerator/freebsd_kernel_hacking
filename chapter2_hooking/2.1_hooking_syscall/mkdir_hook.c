#include <sys/types.h>
#include <sys/param.h>
#include <sys/proc.h>
#include <sys/module.h>
#include <sys/sysent.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/syscall.h>
#include <sys/sysproto.h>

/* mkdir system call hook */
static int mkdir_hook( struct thread *td, void *syscall_args)
{
	struct mkdir_args /* {
		char *path;
		int mode;
	} */ *uap;
	uap = (struct mkdir_args *) syscall_args;

	char path[255];
	size_t done;
	int error;

	error = copyinstr(uap->path, path, 255, &done);
	if (error != 0)
		return(error);

	/* print a debug message */
	printf("The directory \"%s\" will be created with the following permissions: %o\n", path, uap->mode);

	return( sys_mkdir(td, syscall_args) );
}

/* The function called a load/unload */
static int load( struct module *module, int cmd, void *arg)
{
	int error = 0;

	switch(cmd)
	{
		case MOD_LOAD:
			/* replace mkdir with mkdir_hook */
			/* SYS_mkdir is predefined to be the syscall number assigned to mkdir */
			/* sysent[] is an array of structs containing all the information */
			/* we set the sy_call_t struct to our mkdir_hook */
			sysent[SYS_mkdir].sy_call = (sy_call_t *) mkdir_hook;
			break;

		case MOD_UNLOAD:
			/* change everything back to normal */
			sysent[SYS_mkdir].sy_call = (sy_call_t *) sys_mkdir;
			break;

		default:
			error = EOPNOTSUPP;
			break;
	}

	return(error);
}

static moduledata_t mkdir_hook_mod = {
	"mkdir_hook", 		/* module name */
	load,			/* event handler */
	NULL			/* extra data */
};

DECLARE_MODULE( mkdir_hook, mkdir_hook_mod, SI_SUB_DRIVERS, SI_ORDER_MIDDLE);
