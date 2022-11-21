/*
 * patient.c
 *
 * Simulates a patient in the hospital system
 */

#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>     // #define for ND_LOCAL_NODE is in here
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include "patient.h"
#include "shared_memory.h"

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

int main(int argc, char **argv) {

	//todo: request hospital server for server name

	startPatientServer("/patient-name-temp");

	return 0;
}

typedef union
{
	uint16_t type;
	struct _pulse pulse;
	get_shmem_msg_t get_shmem;
	changed_shmem_msg_t changed_shmem;
} recv_buf_t;

void startPatientServer(char* patientServerName){
	// register our name for a channel
	name_attach_t* patientChannel = name_attach(NULL, patientServerName, 0);

	while(1){
		recv_buf_t msg;

		struct _msg_info info;

		int rcvid = MsgReceive(nameChannel->chid, &msg, sizeof(msg), &info);

		pid_t pid = info.pid;

		if (rcvid == 0) {
			//received a pulse

		} else {

			handleMessageFromMonitor(msg, pid);
		}

	}

	name_detach(patientChannel, 0);
	free(patientChannel);
}

void handleMessageFromMonitor(recv_buf_t msg, pid_t pid){
	switch(msg.type){
	case GET_SHMEM_MSG_TYPE: ;

		get_shmem_resp_t rmsg;
		void* ptr;

		create_shared_memory(msg.get_shmem.shared_mem_bytes, pid, &ptr, &rmsg.mem_handle);

		mem_ptr = (char*)ptr;

		//reply to client with the handle
		int result = MsgReply(rcvid, EOK, &rmsg, sizeof(get_shmem_resp_t));

		//todo - set up threads to update shared memory

		break;
	}
}
