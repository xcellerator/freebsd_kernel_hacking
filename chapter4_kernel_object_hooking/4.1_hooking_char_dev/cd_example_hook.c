#include <sys/param.h>
#include <sys/proc.h>
#include <sys/module.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/conf.h>
#include <sys/queue.h>
#include <sys/lock.h>
#include <sys/mutex.h>

#include <fs/devfs/devfs_int.h>

/* cdevp_list contains all active character devices
 * defined in fs/devfs/devfs_int.h */
extern struct cdev_priv_list cdevp_list;

/* d_read_t is a typedef for driver entry switches from a character device switch table
 * They are what get called a read operation is performed on a character device */
d_read_t	read_hook;
d_read_t	*read;

/* read entry point hook */
int read_hook (struct cdev *dev, struct uio *uio, int ioflag)
{
	uprintf("You ever dance with the devil in the pale moonlight?\n");
	
	/* In load() below we grab the memory address of cd_example's real read driver entry switch
	 * before replacing it with this one. Because we made these global variables, we can access
	 * the real read() here without having to include it's definition! */
	return( (*read)(dev, uio, ioflag) );
}

/* The function called at load/unload */
static int load (struct module *module, int cmd, void *arg)
{
	int error = 0;
	struct cdev_priv *cdp;

	switch(cmd)
	{
		case MOD_LOAD:
			/* C */
			mtx_lock(&devmtx);

			/* Replace cd_example's read entry point with the read_hook */
			TAILQ_FOREACH(cdp, &cdevp_list, cdp_list)
			{
				if (strcmp(cdp->cdp_c.si_name, "cd_example") == 0)
				{
					/* We already declared read as a d_read_t
					 * so we can now just grab the memory location
					 * straight out of the character device switch table cdp */
					printf("Found the \"cd_example\" kernel module!\n");
					read = cdp->cdp_c.si_devsw->d_read;
					/* Replace the entry point with our version */
					cdp->cdp_c.si_devsw->d_read = read_hook;
					break;
				}
			}

			mtx_unlock(&devmtx);
			break;

		case MOD_UNLOAD:
			mtx_lock(&devmtx);

			/* Change everything back to normal */
			TAILQ_FOREACH(cdp, &cdevp_list, cdp_list)
			{
				if (strcmp(cdp->cdp_c.si_name, "cd_example") == 0)
				{
					cdp->cdp_c.si_devsw->d_read = read;
					break;
				}
			}

			mtx_unlock(&devmtx);
			break;

		default:
			error = EOPNOTSUPP;
			break;

	}

	return (error);
}

static moduledata_t cd_example_hook_mod = {
	"cd_example_hook", 		/* module name */
	load,				/* event handler */
	NULL				/* extra data */
};

DECLARE_MODULE(cd_example_hook, cd_example_hook_mod, SI_SUB_DRIVERS, SI_ORDER_MIDDLE);

