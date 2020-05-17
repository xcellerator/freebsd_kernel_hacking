/* Compile with -lkvm GCC flag */

#include <stdlib.h>
#include <fcntl.h>
#include <kvm.h>
#include <limits.h>
#include <nlist.h>
#include <stdio.h>
#include <sys/types.h>

/* How far into the "hello" function do we want to search */
/* The function itself if 0x62 bytes long, but we only have 
 * to search through the first 0x40 to find the instruction we want.
 * Ideally, we should calculate the sizze of "hello" for better
 * portability. */
#define SIZE 0x40

/* The instruction we want to patch is 5 bytes long */
/* <hello+0x3f>: e9 d3 ff ff ff		jmpq b7 <hello+0x17> */
unsigned char nop_code[] = "\x90\x90\x90\x90\x90";

int main (int argc, char *argv[])
{
	int i, offset;
	char errbuf[_POSIX2_LINE_MAX];
	/* The file descriptor to locations in kernel memory has it's own type */
	kvm_t *kd;
	/* nlist structures are kernel symbol table entries
	 * nl[] is a null-terminated array of nlist structures
	 * nlist.n_name: name of symbol in memory
	 * nlist.n_value: memory address of symbol */
	struct nlist nl[] = { {NULL}, {NULL}, };
	unsigned char hello_code[SIZE];

	/* Initialize kernel virtual memory access */
	/* kvm_openfiles( const char *execfile, const char *corefile, const char *swapfile, int flags, char *errbuff);
	 * execfile: kernel image to be examined, NULL for currently running kernel
	 * corefile: kernel memory file, NULL for /dev/mem
	 * swapfile: unused, NULL
	 * flags: O_RDONLY, O_WRONLY, or O_RDWR
	 * errbuf: any error encountered is written to this buffer */
	kd = kvm_openfiles(NULL, NULL, NULL, O_RDWR, errbuf);
	if (kd == NULL)
	{
		fprintf(stderr, "ERROR: %s\n", errbuf);
		exit(-1);
	}

	/* The name of the symbol we're looking for */
	nl[0].n_name = "hello";

	/* Find the address of hello */
	/* kvm_nlist() iterates through nl[] and looks up each symbol
	 * if it finds the symbol, it assigns nlist.n_value to the memory address,
	 * otherwise sets it to 0. */
	if (kvm_nlist(kd, nl) < 0)
	{
		/* kvm_geterr() returns the latest error encountered on a kvm_t descriptor */
		fprintf(stderr, "ERROR: %s\n", kvm_geterr(kd));
		exit(-1);
	}

	if (!nl[0].n_value)
	{
		/* If nl[0].n_value = 0, then kvm_nlist() couldn't find the symbol in memory */
		fprintf(stderr, "ERROR: Symbol %s not found\n", nl[0].n_name);
		exit(-1);
	}

	printf("Found \"hello\" at address: 0x%x.\n", nl[0].n_value);

	/* Save a copy of hello */
	/* kvm_read(kvm_t *kd, unsigned long addr, void *buf, size_t nbytes);
	 * will read nbytes from addr in kernel descriptor kd, saving the result in buf */
	if (kvm_read(kd, nl[0].n_value, hello_code, SIZE) < 0)
	{
		fprintf(stderr, "ERROR: %s\n", kvm_geterr(kd));
		exit(-1);
	}

	printf("Read 0x%x bytes from \"hello\" at address 0x%x.\n", SIZE, nl[0].n_value);

	/* Search through hello for the jns instruction */
	/* 0xe9 is the jmpq instruction that we want to patch out */
	for ( i = 0 ; i < SIZE ; i++ )
	{
		if(hello_code[i] == 0xe9)
		{
			offset = i;
			printf("Found jmpq instruction at offset: 0x%x\n", offset);
			break;
		}
	}

	/* Patch hello */
	/* NOTE: We only write sizeof(nop_code) - 1, to avoid the NULL automatically appended to nop_code */
	if (kvm_write(kd, nl[0].n_value + offset, nop_code, sizeof(nop_code) - 1) < 0)
	{
		fprintf(stderr, "ERROR: %s\n", kvm_geterr(kd));
		exit(-1);
	}
	else
	{
		printf("Patched memory address 0x%x with %x.\n", nl[0].n_value + offset, nop_code);
	}

	/* Close kd */
	if (kvm_close(kd) < 0)
	{
		fprintf(stderr, "ERROR: %s\n", kvm_geterr(kd));
		exit(-1);
	}

	exit(0);
}
