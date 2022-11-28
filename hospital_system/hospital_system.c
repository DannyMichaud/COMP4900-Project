/*
 * hospital_system.c
 *
 * Used to organize the hospital
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
#include "hospital_system.h"
#include "hospital_system_address.h"
#include "../shared_memory/shared_memory.h"


int main(int argc, char **argv) {

	name_attach_t* systemChannel = malloc(sizeof(name_attach_t));

	startServer(&systemChannel);

	mainLoop(&systemChannel);

	free(systemChannel);
	return 0;
}

void startServer(name_attach_t** channel){

	*channel = name_attach(NULL, HOSPITAL_SERVER_NAME, 0);

}

void mainLoop(name_attach_t** channel){

	monitor_array_t monitorList = {0, NULL};
	monitorList.monitors = malloc(sizeof(monitor_data_t));

	while(1){

		hospital_system_msg_to_t msg;

		struct _msg_info info;

		int rcvid = MsgReceive((*channel)->chid, &msg, sizeof(msg), &info);

		hospital_system_msg_from_t rmsg;

		printf("Received msg, rcvid %d\n", rcvid);

		if(rcvid == -1){
			continue;
		}

		//handle message depending on whether it came from patient or monitor
		switch(msg.messageSender){
		case SOURCE_PATIENT:
			handlePatientMessage(&msg, &rmsg, &monitorList);
			break;
		case SOURCE_MONITOR:
			handleMonitorMessage(&msg, &rmsg, &monitorList, info.pid);
			break;
		default:
			break;
		}

		//send message back to client
		int result = MsgReply(rcvid, EOK, &rmsg, sizeof(rmsg));

		//todo: extra handling depending on message sent back?
	}

	free(monitorList.monitors);
}

int handlePatientMessage(hospital_system_msg_to_t* msg, hospital_system_msg_from_t* rmsg, monitor_array_t* monitorList){

	//upon patient connection, find it a freed up monitor and
	switch(msg->messageType){
	case HS_MSG_CONNECT:

		for(int i = 0; i < monitorList->length; i++){

			if(!monitorList->monitors[i].inUse){
				//todo: generate random server name


				monitorList->monitors[i].inUse = 1;
				return monitorList->monitors[i].id;
			}

		}

		break;
	default:
		break;
	}

	return -1;
}

void handleMonitorMessage(hospital_system_msg_to_t* msg, hospital_system_msg_from_t* rmsg, monitor_array_t* monitorList, pid_t monitor_pid){

	void* shmem_ptr;

	shm_handle_t monitor_handle;

	switch(msg->messageType){
	case HS_MSG_CONNECT:

		//create shared memory to be used in hospital system -> monitor un-prompted communication
		create_shared_memory(HS_MONITOR_SHMEM_SIZE, monitor_pid, &shmem_ptr, &monitor_handle);

		//write default value to patient name section
		write_shmem(shmem_ptr, "No patient\0", HS_SHMEM_OFFSET_PATIENT_NAME, 20);

		printf("%s\n", read_shmem(shmem_ptr, HS_SHMEM_OFFSET_PATIENT_NAME));



		//NOTE: ANY new shared memory values we want to add MUST be initialized here!!!! Or the monitor will segfault!!1!111!

		//track new monitor with id equal to previous length of array
		monitorList->monitors = realloc(monitorList->monitors, (monitorList->length + 1)*sizeof(monitor_data_t));
		monitorList->monitors[monitorList->length] = (monitor_data_t){monitorList->length, 0, shmem_ptr};
		monitorList->length++;

		//send back id and shmem handle to monitor for future reference
		rmsg->messageReplyType = HS_REPLY_SETUP;
		rmsg->data.int_data = monitorList->monitors[monitorList->length-1].id;
		rmsg->data2.shmem_handle = monitor_handle;

		break;
	default:
		break;
	}
}


