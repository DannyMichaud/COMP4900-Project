/*
 * patient.c
 *
 * Simulates a patient in the hospital system
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
#include "patient.h"

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

typedef union
{
	uint16_t type;
	struct _pulse pulse;
	get_shmem_msg_t get_shmem;
} recv_buf_t;

int main(int argc, char **argv) {

	//todo: request hospital server for server name

	startPatientServer("patient-name-temp");


	return 0;
}

void startPatientServer(char* patientServerName){
	// register our name for a channel

	name_attach_t* patientChannel = name_attach(NULL, patientServerName, 0);

	while(1){
		recv_buf_t msg;

		struct _msg_info info;

		int rcvid = MsgReceive(patientChannel->chid, &msg, sizeof(msg), &info);

		pid_t pid = info.pid;

		if (rcvid == 0) {
			//received a pulse

		} else {

			handleMessageFromMonitor(msg, pid, rcvid);
		}

	}

	name_detach(patientChannel, 0);
	free(patientChannel);
}


void handleMessageFromMonitor(recv_buf_t msg, pid_t pid, int rcvid){
	switch(msg.type){
	case GET_SHMEM_MSG_TYPE: ;

		get_shmem_resp_t rmsg;
		void* ptr;

		create_shared_memory(msg.get_shmem.shared_mem_bytes, pid, &ptr, &rmsg.mem_handle);


		//reply to client with the handle
		int result = MsgReply(rcvid, EOK, &rmsg, sizeof(get_shmem_resp_t));

		//todo - set up threads to update shared memory
		pthread_t threads[5];

		initVitalThreads(threads);

		break;
	}
}

void initVitalThreads(pthread_t* threads){

	patient_vital_thrinfo_t* vT1 = malloc(sizeof(patient_vital_thrinfo_t));
	*vT1 = (patient_vital_thrinfo_t){HEARTBEAT, PATIENT_SHMEM_OFFSET_HEARTBEAT };
	patient_vital_thrinfo_t* vT2 = malloc(sizeof(patient_vital_thrinfo_t));
	*vT2 = (patient_vital_thrinfo_t){BLOOD_PRESSURE_SYSTOLIC, PATIENT_SHMEM_OFFSET_BLOOD_PRESSURE_SYSTOLIC };
	patient_vital_thrinfo_t* vT3 = malloc(sizeof(patient_vital_thrinfo_t));
	*vT3 = (patient_vital_thrinfo_t){BLOOD_PRESSURE_DIATOLIC, PATIENT_SHMEM_OFFSET_BLOOD_PRESSURE_DIATOLIC};
	patient_vital_thrinfo_t* vT4 = malloc(sizeof(patient_vital_thrinfo_t));
	*vT4 = (patient_vital_thrinfo_t){TEMPERATURE, PATIENT_SHMEM_OFFSET_TEMPERATURE};
	patient_vital_thrinfo_t* vT5 = malloc(sizeof(patient_vital_thrinfo_t));
	*vT5 = (patient_vital_thrinfo_t){RESPIRATION, PATIENT_SHMEM_OFFSET_RESPIRATION};
	patient_vital_thrinfo_t* vT6 = malloc(sizeof(patient_vital_thrinfo_t));
	*vT6 = (patient_vital_thrinfo_t){OXYGEN_SATURATION, PATIENT_SHMEM_OFFSET_OXYGEN_SATURATION};

	//create thread for each vital
	pthread_create(&threads[0], NULL,(void*) &updateVitalOnInterval, vT1);
	pthread_create(&threads[1], NULL,(void*) &updateVitalOnInterval, vT2);
	pthread_create(&threads[2], NULL,(void*) &updateVitalOnInterval, vT3);
	pthread_create(&threads[3], NULL,(void*) &updateVitalOnInterval, vT4);
	pthread_create(&threads[4], NULL,(void*) &updateVitalOnInterval, vT5);
	pthread_create(&threads[5], NULL,(void*) &updateVitalOnInterval, vT6);

	pthread_join(threads[5], NULL);
}


//Will create timer, each interval will get value for corresponding vital, and write to corresponding shmem location...
void updateVitalOnInterval(patient_vital_thrinfo_t* vitalInfo){

	printf("Test: Vital type %d, offset %d\n", vitalInfo->vitalType, vitalInfo->offset);

}

