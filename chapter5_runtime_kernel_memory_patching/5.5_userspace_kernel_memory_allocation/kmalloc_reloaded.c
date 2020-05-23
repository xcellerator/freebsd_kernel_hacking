#include <fcntl.h>
#include <kvm.h>
#include <limits.h>
#include <nlist.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/module.h>

/* kernel memory allocation (kmalloc) function code */
/* This is from the disassembly of kmalloc.ko 
 * Note that there are 6 addresses at offsets 78, 88, 139, 159, 169, and 217 
 * that are currently set to \x00\x00\x00\x00. This is because they are
 * relative jumps that we need to calculate and overwrite at runtime. */
unsigned char kmalloc[] =
	"\x55"                   	/* push   %rbp			*/
	"\x48\x89\xe5"             	/* mov    %rsp,%rbp		*/
	"\x48\x83\xec\x50"          	/* sub    $0x50,%rsp		*/
	"\x31\xc0"                	/* xor    %eax,%eax		*/
	"\x88\xc1"                	/* mov    %al,%cl		*/
	"\x48\x89\x7d\xf8"          	/* mov    %rdi,-0x8(%rbp)	*/
	"\x48\x89\x75\xf0"          	/* mov    %rsi,-0x10(%rbp)	*/
	"\x48\x8b\x75\xf0"          	/* mov    -0x10(%rbp),%rsi	*/
	"\x48\x89\x75\xe8"          	/* mov    %rsi,-0x18(%rbp)	*/
	"\x48\x8b\x75\xe8"          	/* mov    -0x18(%rbp),%rsi	*/
	"\x48\x8b\x36"             	/* mov    (%rsi),%rsi		*/
	"\x48\x89\x75\xc8"          	/* mov    %rsi,-0x38(%rbp)	*/
	"\xf6\xc1\x01"             	/* test   $0x1,%cl 		*/
	"\x0f\x85\x05\x00\x00\x00"    	/* jne    d5 <kmalloc+0x35>	*/
	"\xe9\x63\x00\x00\x00"       	/* jmpq   138 <kmalloc+0x98>	*/
	"\x31\xc0"                	/* xor    %eax,%eax		*/
	"\x88\xc1"                	/* mov    %al,%cl		*/
	"\xf6\xc1\x01"             	/* test   $0x1,%cl		*/
	"\x0f\x85\x05\x00\x00\x00"    	/* jne    e7 <kmalloc+0x47>	*/
	"\xe9\x51\x00\x00\x00"       	/* jmpq   138 <kmalloc+0x98>	*/
	"\x48\x8b\x7d\xc8"          	/* mov    -0x38(%rbp),%rdi	*/
	"\x48\xc7\xc6\x00\x00\x00\x00" 	/* mov    $0x0,%rsi		*/
					/* (78) R_X86_64_32S	M_TEMP	*/
	"\xba\x01\x40\x00\x00"       	/* mov    $0x4001,%edx		*/
	"\xe8\x00\x00\x00\x00"       	/* callq  fc <kmalloc+0x5c>	*/
					/* (88) R_X86_64_PLT32 malloc	*/
	"\x48\x89\x45\xd0"          	/* mov    %rax,-0x30(%rbp)	*/
	"\x48\x83\x7d\xd0\x00"       	/* cmpq   $0x0,-0x30(%rbp)	*/
	"\x0f\x95\xc1"             	/* setne  %cl			*/
	"\x80\xe1\x01"             	/* and    $0x1,%cl		*/
	"\x0f\xb6\xd1"             	/* movzbl %cl,%edx		*/
	"\x48\x63\xc2"             	/* movslq %edx,%rax		*/
	"\x48\x83\xf8\x00"          	/* cmp    $0x0,%rax		*/
	"\x0f\x84\x18\x00\x00\x00"    	/* je     133 <kmalloc+0x93>	*/
	"\x31\xc0"                	/* xor    %eax,%eax		*/
	"\x48\x8b\x7d\xd0"          	/* mov    -0x30(%rbp),%rdi	*/
	"\x48\x8b\x55\xc8"          	/* mov    -0x38(%rbp),%rdx	*/
	"\x31\xf6"                	/* xor    %esi,%esi		*/
	"\x89\x45\xbc"             	/* mov    %eax,-0x44(%rbp)	*/
	"\xe8\x00\x00\x00\x00"       	/* callq  12f <kmalloc+0x8f>	*/
					/* (139) R_X86_64_PLT32 memset  */
	"\x48\x89\x45\xb0"          	/* mov    %rax,-0x50(%rbp)	*/
	"\xe9\x19\x00\x00\x00"       	/* jmpq   151 <kmalloc+0xb1>	*/
	"\x48\x8b\x7d\xc8"          	/* mov    -0x38(%rbp),%rdi	*/
	"\x48\xc7\xc6\x00\x00\x00\x00" 	/* mov    $0x0,%rsi		*/
					/* (159) R_X86_64_32S M_TEMP	*/
	"\xba\x01\x40\x00\x00"       	/* mov    $0x4001,%edx		*/
	"\xe8\x00\x00\x00\x00"       	/* callq  14d <kmalloc+0xad>	*/
					/* (169) R_X86_64_PLT32 malloc	*/
	"\x48\x89\x45\xd0"          	/* mov    %rax,-0x30(%rbp)	*/
	"\x48\x8b\x45\xd0"          	/* mov    -0x30(%rbp),%rax	*/
	"\x48\x89\x45\xc0"          	/* mov    %rax,-0x40(%rbp)	*/
	"\x48\x8b\x45\xc0"          	/* mov    -0x40(%rbp),%rax	*/
	"\x48\x89\x45\xd8"          	/* mov    %rax,-0x28(%rbp)	*/
	"\x48\x8d\x45\xd8"          	/* lea    -0x28(%rbp),%rax	*/
	"\x48\x8b\x4d\xe8"          	/* mov    -0x18(%rbp),%rcx	*/
	"\x48\x8b\x49\x08"          	/* mov    0x8(%rcx),%rcx	*/
	"\xba\x08\x00\x00\x00"       	/* mov    $0x8,%edx		*/
	"\x48\x89\xc7"             	/* mov    %rax,%rdi		*/
	"\x48\x89\xce"             	/* mov    %rcx,%rsi		*/
	"\xe8\x00\x00\x00\x00"       	/* callq  17d <kmalloc+0xdd>	*/
					/* (217) R_X86_64_PLT32 copyout */
	"\x89\x45\xe4"             	/* mov    %eax,-0x1c(%rbp)	*/
	"\x8b\x45\xe4"             	/* mov    -0x1c(%rbp),%eax	*/
	"\x48\x83\xc4\x50"          	/* add    $0x50,%rsp		*/
	"\x5d"                   	/* pop    %rbp			*/
	"\xc3";                  	/* retq   			*/
/*
 * The relative address of the instructions following the call statements
 * within kmalloc. Note that these are just the offsets of the relative
 * addresses above +4 (to allow for the 4-byte addresses we are going to
 * overwrite.
 */

#define OFFSET_MALLOC1	0x5c
#define OFFSET_MEMSET	0x8f
#define OFFSET_MALLOC2	0xad
#define OFFSET_COPYOUT	0xdd

int main (int argc, char *argv[])
{
	int i;
	char errbuf[_POSIX2_LINE_MAX];
	kvm_t *kd;
	struct nlist nl[] = { {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, };
	unsigned char mkdir_code[sizeof(kmalloc)];
	unsigned long addr;

	if (argc != 2)
	{
		printf("Usage: %s <size>\n", argv[0]);
		exit(0);
	}

	/* Initialize kernel virtual memory access */
	kd = kvm_openfiles(NULL, NULL, NULL, O_RDWR, errbuf);
	if (kd == NULL)
	{
		fprintf(stderr, "ERROR: %s\n", errbuf);
		exit(-1);
	}

	nl[0].n_name = "sys_mkdir";
	nl[1].n_name = "M_TEMP";
	nl[2].n_name = "malloc";
	nl[3].n_name = "memset";
	nl[4].n_name = "copyout";

	/* Find the address of sys_mkdir, M_TEMP, malloc, memset, and copyout */
	if (kvm_nlist(kd, nl) < 0)
	{
		fprintf(stderr, "ERROR: %s\n", kvm_geterr(kd));
		exit(-1);
	}

	for ( i = 0 ; i < 5 ; i++ )
	{
		if (!nl[i].n_value)
		{
			fprintf(stderr, "ERROR: Symbol %s not found\n", nl[i].n_name);
			exit(-1);
		}
	}

	/*
	 * Patch the kmalloc function code to contain the correct addresses
	 * for M_TEMP, malloc, copyout, and memset.
	 */
	/* M_TEMP */
	*(unsigned int *)&kmalloc[78] = nl[1].n_value;
	/* malloc */
	*(unsigned int *)&kmalloc[88] = nl[2].n_value - (nl[0].n_value + OFFSET_MALLOC1);
	/* memset */
	*(unsigned int *)&kmalloc[139] = nl[3].n_value - (nl[0].n_value + OFFSET_MEMSET);
	/* M_TEMP */
	*(unsigned int *)&kmalloc[159] = nl[1].n_value;
	/* malloc */
	*(unsigned int *)&kmalloc[169] = nl[2].n_value - (nl[0].n_value + OFFSET_MALLOC2);
	/* copyout*/
	*(unsigned int *)&kmalloc[217] = nl[4].n_value - (nl[0].n_value + OFFSET_COPYOUT);

	/* Save sizeof(kmalloc) bytes of mkdir
	 * This way, we can just write back the portion that was 
	 * overwritten when we're done, preserving system stability! */
	if (kvm_read(kd, nl[0].n_value, mkdir_code, sizeof(kmalloc)) < 0)
	{
		fprintf(stderr, "ERROR: %s\n", kvm_geterr(kd));
		exit(-1);
	}

	/* Overwrite mkdir with kmalloc */
	if (kvm_write(kd, nl[0].n_value, kmalloc, sizeof(kmalloc)) < 0)
	{
		fprintf(stderr, "ERROR: %s\n", kvm_geterr(kd));
		exit(-1);
	}
	else
	{
		printf("[*] Wrote %d bytes of kmalloc to 0x%x\n", sizeof(kmalloc), nl[0].n_value);
	}

	/* Allocate kernel memory */
	syscall(136, (unsigned long)atoi(argv[1]), &addr);
	printf("[+] Allocated %d bytes of kernel memory at 0x%x\n", atoi(argv[1]), addr);

	/* Restore mkdir */
	if (kvm_write(kd, nl[0].n_value, mkdir_code, sizeof(kmalloc)) < 0)
	{
		fprintf(stderr, "ERROR: %s\n", kvm_geterr(kd));
		exit(-1);
	}
	else
	{
		printf("[*] Wrote %d bytes of sys_mkdir to 0x%x\n", sizeof(kmalloc), nl[0].n_value);
	}

	/* Close kd */
	if (kvm_close(kd) < 0)
	{
		fprintf(stderr, "ERROR: %s\n", kvm_geterr(kd));
		exit(-1);
	}

	exit(0);
}
