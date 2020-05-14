#include <sys/param.h>
#include <sys/proc.h>
#include <sys/module.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/mbuf.h>
#include <sys/protosw.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip_var.h>
#include <netinet/ip_options.h>

#define TRIGGER "Shiny."

extern struct protosw inetsw[];
pr_input_t icmp_input_hook;

/* icmp_input hook */
int icmp_input_hook(struct mbuf **mp, int *off, int proto)
{
	/* Our icmp packet structure */
	struct icmp *icp;
	/* FreeBSD changed the function prototype arguments, so we derefrence **mp to *m */
	struct mbuf *m = *mp;
	/* We set hlen to the received ICMP message's IP header length (*off in args) */
	int hlen = *off;
	/* First, get the IP packet struct out of mbuf with mtod() */
	struct ip *ip = mtod(m, struct ip *);
	/* We need to check whether we've got an ICMP message, so get the length of the data field from the IP header  */
	int icmplen = ntohs(ip->ip_len) - hlen;
	/* Check whether the length of the data field is long enough and whether the data in m is accessible */
	int i = hlen + min(icmplen, ICMP_ADVLENMIN);
	if ( m->m_len < i && (m = m_pullup(m,i)) == NULL )
		/* Exit without tampering */
		return(IPPROTO_DONE);


	/* Locate ICMP message within the buffer m */
	m->m_len -= hlen;
	/* We increase m_data by hlen, to skip the IP header */
	m->m_data += hlen;

	/* Extract ICMP message */
	/* mtod() converts mbuf pointer to data pointer */
	icp = mtod(m, struct icmp *);

	/* Restore m */
	/* Reverse the changes we made to the mbuf once we've extracted the ICMP message */
	m->m_len += hlen;
	m->m_data -= hlen;

	/* Is this the ICMP message we're looking for? */
	if (	icp->icmp_type == ICMP_REDIRECT &&
		icp->icmp_code == ICMP_REDIRECT_TOSHOST &&
		strncmp(icp->icmp_data, TRIGGER, 6) == 0	)
		/* ICMP_REDIRECT is for short routes and ICMP_REDIRECT_TOSHOST is for tos and host */
			printf("Let's be bad guys.\n");
	else
			/* Handle the packet normally */
			icmp_input(mp, off, 0);

	return(IPPROTO_DONE);
}

/* The function called at load/unload */
static int load( struct module *module, int cmd, void *arg)
{
	int error = 0;

	switch(cmd)
	{
		case MOD_LOAD:
			/* Replace icmp_input with icmp_input_hook */
			/* Register icmp_input_hook as the .pr_input entry point in the ICMP switch table */
			inetsw[ip_protox[IPPROTO_ICMP]].pr_input = icmp_input_hook;
			break;

		case MOD_UNLOAD:
			/* Change everything back to normal */
			inetsw[ip_protox[IPPROTO_ICMP]].pr_input = icmp_input;
			break;

		default:
			error = EOPNOTSUPP;
			break;

	}

	return(error);
}

static moduledata_t icmp_input_hook_mod = {
	"icmp_input_hook", 	/* module name */
	load,			/* event handler */
	NULL			/* extra data */
};

DECLARE_MODULE( icmp_input_hook, icmp_input_hook_mod, SI_SUB_DRIVERS, SI_ORDER_MIDDLE);
