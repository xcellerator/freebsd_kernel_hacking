#include <sys/param.h>
#include <sys/proc.h>
#include <sys/module.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/conf.h>
#include <sys/uio.h>


/* Function prototypes */
d_open_t	open;
d_close_t	close;
d_read_t	read;
d_write_t	write;

/* The cdevsw structure is the character device switch table */
static struct cdevsw cd_example_cdevsw = {
	.d_version = 	D_VERSION,
	.d_open = 	open,
	.d_close = 	close,
	.d_read = 	read,
	.d_write = 	write,
	.d_name = 	"cd_example"
};

/* Set up a buffer that we can read/write to/from */
static char buf[512+1];
static size_t len;

/* After naming these functions in the character device switch table cdevsw, we have to define them properly */
/* open() will just initialize the buffer in memory */
/* note that buf resides in KERNEL memory, NOT in user memory */
int open (struct cdev *dev, int flag, int otyp, struct thread *td)
{
	/* Initialize character buffer */
	memset( &buf, '\0', 512);
	len = 0;

	return(0);
}

/* close does nothing, there isn't any special operation we need to perform */
int close (struct cdev *dev, int flag, int otyp, struct thread *td)
{
	return(0);
}

/* copy a string from user space (uio->uio_iov->iov_base) to the buf buffer in kernel space  */
int write (struct cdev *dev, struct uio *uio, int ioflag)
{
	int error = 0;

	/*
	 * Take in a character string, save it in buf.
	 * Note: The proper way to transfer data between buffers and I/O
	 * vectors that cross the user/kernel space boundary is with
	 * uiomove(), but this way is shorter. For more on device driver I/O
	 * routines, see the uio(9) manual page.
	 */
	error = copyinstr(uio->uio_iov->iov_base, &buf, 512, &len);
	if (error != 0)
		uprintf("Write to \"cd_example\" failed.\n");

	return(error);
}

/* Same as write(), but in reverse */
int read (struct cdev *dev, struct uio *uio, int ioflag)
{
	int error = 0;

	if (len <= 0)
		error = -1;

	else
		/* Return the saved character string to userland. */
		copystr(&buf, uio->uio_iov->iov_base, 513, &len);

	return(error);
}

/* Reference to the device in DEVFS. */
static struct cdev *sdev;

/* The function called at load/unload. */
static int load (struct module *module, int cmd, void *arg)
{
	int error = 0;

	switch(cmd)
	{
		case MOD_LOAD:
			/* make_dev() creates the device file in /dev */
			sdev = make_dev(&cd_example_cdevsw, 0, UID_ROOT, GID_WHEEL, 0600, "cd_example");
			uprintf("Character device loaded.\n");
			break;

		case MOD_UNLOAD:
			/* destroy_dev() removes the device file from /dev */
			destroy_dev(sdev);
			uprintf("Character device unloaded.\n");
			break;

		default:
			error = EOPNOTSUPP;
			break;
	}

	return(error);
}

/* DEV_MODULE is a macro wrapper around DECLARE_MODULE especially for character device drivers */
DEV_MODULE(cd_example, load, NULL);
