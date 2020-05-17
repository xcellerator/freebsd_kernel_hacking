/* Compile with -lkvm flag */

#include <stdlib.h>
#include <fcntl.h>
#include <kvm.h>
#include <limits.h>
#include <nlist.h>
#include <stdio.h>
#include <sys/types.h>

/* We know that the function is 0x62 bytes long from the disassembly */
#define SIZE 0x62

/* Replacement Code */
unsigned char nop_code[] = "\x90\x90\x90\x90\x90";

int main(int argc, char *argv[])
{
	int i, jmpq_offset;
	int call_offset = 0;
	char errbuf[_POSIX2_LINE_MAX];
	kvm_t *kd;
	struct nlist nl[] = { {NULL}, {NULL}, {NULL}, };
	unsigned char hello_code[SIZE], call_operand[4];

	/* Initialize kernel virtual memory access */
	kd = kvm_openfiles(NULL, NULL, NULL, O_RDWR, errbuf);
	if (kd == NULL)
	{
		fprintf(stderr, "ERROR: %s\n", errbuf);
		exit(-1);
	}

	nl[0].n_name = "hello";
	nl[1].n_name = "uprintf";

	/* Find addresses of hello and uprintf */
	if (kvm_nlist(kd, nl) < 0)
	{
		fprintf(stderr, "ERROR: %s\n", kvm_geterr(kd));
		exit(-1);
	}

	if (!nl[0].n_value)
	{
		fprintf(stderr, "ERROR: Symbol %s not found\n", nl[0].n_name);
		exit(-1);
	}

	if (!nl[1].n_value)
	{
		fprintf(stderr, "ERROR: Symbol %s not found\n", nl[1].n_name);
		exit(-1);
	}

	/* Save a copy of hello */
	if (kvm_read(kd, nl[0].n_value, hello_code, SIZE) < 0 )
	{
		fprintf(stderr, "ERROR: %s\n", kvm_geterr(kd));
		exit(-1);
	}

	/* Search through hello for the jmpq and callq instructions */
	/* Note that we obtain relative addresses in this process */
	for (i = 0 ; i < SIZE ; i++)
	{
		if (hello_code[i] == 0xe9)
			jmpq_offset = i;
		/* There are 2 calls to printf in "hello", we only want to patch the first one */
		if (hello_code[i] == 0xe8 && call_offset == 0)
			call_offset = i;
	}

	/* Calculate the call statement operand */
	/* call_operand = address_of_uprintf - address_of_instruction_after_callq
	 * 		= address_of_uprintf - <hello+0xf6> */
	/* 4 */
	*(unsigned long *)&call_operand[0] = nl[1].n_value - (nl[0].n_value + call_offset + 5);

	/* Patch hello */
	if (kvm_write(kd, nl[0].n_value + jmpq_offset, nop_code, sizeof(nop_code) - 1) < 0 )
	{
		fprintf(stderr, "ERROR: %s\n", kvm_geterr(kd));
		exit(-1);
	}

	/* Overwrite the callq instruction with the new call_operand */
	/* nl[0].n_value + call_offset is the memory address of the callq instruction
	 * we add +1 to skip past the 0xe8 byte and only modify the short address we jump to */
	if (kvm_write(kd, nl[0].n_value + call_offset + 1, call_operand, sizeof(call_operand)) < 0 )
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

	exit(0);
}
