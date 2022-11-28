/*
 * monitor.c
 *
 * Used to monitor patient vitals, and report to hospital system if things are going wrong...
 */

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
#include "monitor.h"
#include "../shared_memory/shared_memory.h"

int main(int argc, char **argv) {

	//todo: receive server name from hospital server

	connectToPatient("patient-name-temp");

	return 0;
}

void connectToPatient(char* patientServerName){

	/* find our server to get a coid*/
	int coid = name_open(patientServerName, 0);

	/* request for shared memory */
	get_shmem_msg_t get_msg;
	get_shmem_resp_t get_msg_reply;

	get_msg.type = GET_SHMEM_MSG_TYPE;
	get_msg.shared_mem_bytes = PATIENT_SHMEM_SIZE;

	//send the message to the server and get a reply
	int status = MsgSend(coid, &get_msg, sizeof(get_shmem_msg_t), &get_msg_reply, sizeof(get_shmem_resp_t));

    //open handle to get fd
	int fd = shm_open_handle(get_msg_reply.mem_handle, O_RDWR);

    //map the shared memory
	void *patient_shmem_ptr = mmap(NULL, PATIENT_SHMEM_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

	/* once mapped, we don't need the fd anymore */
	close(fd);

	printf("Response code for get message: %d\n", status);

	sleep(3);

	//just a test
	int heartRate = read_shmem_int(patient_shmem_ptr, PATIENT_SHMEM_OFFSET_HEARTBEAT);

	printf("Test reading heartbeat from patient %d\n", heartRate);

	munmap(patient_shmem_ptr, 4096);
}
