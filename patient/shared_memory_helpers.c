#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <process.h>
#include <string.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>


/* shared_memory_helpers.c
 *
 * Helpers for reading/writing to any given shared memory block
 * Can read/write ints/floats (will be written to as strings)
 *
 *
 * */

void write_shmem_int(void* shmem_ptr, int int_val, uint16_t offset, uint8_t range){
	int length = snprintf(NULL, 0, "%d", int_val);

	char* int_string = malloc(length+1);

	snprintf(int_string, length + 1, "%d", int_val);

	write_shmem(shmem_ptr, int_string, offset, range);
}

void write_shmem_float(void* shmem_ptr, float float_val, uint16_t offset, uint8_t range){

	int length = snprintf(NULL, 0, "%f", float_val);

	char* float_string = malloc(length+1);

	snprintf(float_string, length + 1, "%f", float_val);

	write_shmem(shmem_ptr, float_string, offset, range);
}

void write_shmem(void* shmem_ptr, char* output, uint16_t offset, uint8_t range){

	printf("%s %d %d\n", output, offset, strlen(output));

	memcpy(shmem_ptr + offset, output, strlen(output)+1);

}

