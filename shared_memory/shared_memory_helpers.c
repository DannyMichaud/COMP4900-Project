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
#include "shared_memory.h"


/* shared_memory_helpers.c
 *
 * Helpers for reading/writing to any given shared memory block
 * Can read/write ints/floats (will be written to as strings)
 *
 *
 * */

/* create a secured shared-memory object, updating a handle to it. */

int create_shared_memory(unsigned nbytes, int client_pid, void **ptr, shm_handle_t *handle) {

	/* create an anonymous shared memory object */
	int fd = shm_open(SHM_ANON, O_RDWR|O_CREAT, 0600);

	/* allocate the memory for the object */
	ftruncate(fd, nbytes);

	/* get a local mapping to the object */
	*ptr = mmap(NULL, nbytes, PROT_NOCACHE|PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	/* print fd and client pid */
	printf("Fd %d, client pid %d\n", fd, client_pid);

	/* get a handle for the client to map the object */
	shm_create_handle(fd, client_pid, O_RDWR, handle, NULL);

	/* we no longer need the fd, so cleanup */
	close(fd);

	return 0;
}

/* Writing */

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

/* Reading */

int read_shmem_int(void *shmem_ptr, uint16_t offset){

	char* int_string = read_shmem(shmem_ptr, offset);

	int return_val = atoi(int_string);

	free(int_string);

	return return_val;
}

float read_shmem_float(void* shmem_ptr, uint16_t offset){

	char* float_string = read_shmem(shmem_ptr, offset);

	float return_val = atof(float_string);

	free(float_string);

	return return_val;
}

char* read_shmem(void* shmem_ptr, uint16_t offset){

	int length = snprintf(NULL, 0, "%s", shmem_ptr + offset);

	char* return_string = malloc(length+1);

	snprintf(return_string, length + 1, "%s", shmem_ptr + offset);

	return return_string;
}
