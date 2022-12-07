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
#include <spawn.h>
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

	printf("Hospital system starting...\n");

	startServer(&systemChannel);


	if(!(argc >= 2 && strcmp(argv[1],"noSpawn") == 0)){
		printf("Spawning monitors...\n");
		spawnMonitors();
	}
	else {
		printf("Spawning of monitors disabled\n");
	}

	printf("Entering main loop...\n");

	mainLoop(&systemChannel);

	free(systemChannel);

	return 0;
}

void startServer(name_attach_t** channel){

	*channel = name_attach(NULL, HOSPITAL_SERVER_NAME, 0);

}

void spawnMonitors(){

	struct inheritance inherit;
	inherit.flags = 0;

	//needed to hide output of monitors
	int fd_map[] = {SPAWN_FDCLOSED};
	//just 5 monitors
	for(int i = 0; i < 5; i++){

		char *args[] = {"monitor", NULL};
		if ((spawn("monitor", 1, fd_map, &inherit, args, environ)) == -1)
		{
			printf("ERROR: Could not spawn monitor");
		}

	}

}

void mainLoop(name_attach_t** channel){

	monitor_array_t monitorList = {0, NULL};
	monitorList.monitors = malloc(sizeof(monitor_data_t));

	while(1){

		hospital_system_msg_to_t msg;

		struct _msg_info info;

		int rcvid = MsgReceive((*channel)->chid, &msg, sizeof(msg), &info);

		hospital_system_msg_from_t rmsg;

		printf("Received message, rcvid %d\n", rcvid);

		if(rcvid == -1 || rcvid == 0){
			printf("Disconnect/error message received\n");
			continue;
		}

		//handle message depending on whether it came from patient or monitor
		switch(msg.messageSender){
		case SOURCE_PATIENT:
			printf("Handling message received from patient\n");
			handlePatientMessage(&msg, &rmsg, &monitorList);
			break;
		case SOURCE_MONITOR:
			printf("Handling message received from monitor\n");
			handleMonitorMessage(&msg, &rmsg, &monitorList, info.pid);
			break;
		case SOURCE_SHUTDOWN:
			printf("Handling shutdown\n");
			handleShutdown(channel,&monitorList);
			break;
		default:
			break;
		}

		printf("Message handled, replying\n");

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
		printf("Handling connection request from patient\n");

		for(int i = 0; i < monitorList->length; i++){

			if(!monitorList->monitors[i].inUse){

				//generate random server name
				char* patientServerName = generatePatientServerName();

				printf("Patient server name: %s\n", patientServerName);

				printf("Patient will be connected to monitor %d\n", monitorList->monitors[i].id);

				//set up reply with server name to patient
				rmsg->messageReplyType = HS_REPLY_SERVER_NAME;
				strcpy(rmsg->data.string_data, patientServerName);

				//also write to monitor's shared memory so it can start reading from the patient
				write_shmem(monitorList->monitors[i].shmem_ptr, patientServerName, HS_SHMEM_OFFSET_PATIENT_NAME, 20);

				free(patientServerName);

				monitorList->monitors[i].inUse = 1;
				return monitorList->monitors[i].id;
			}

		}

		//if no monitor found, send error message to patient which will cause it to shutdown
		rmsg->messageReplyType = HS_REPLY_SERVER_NAME;
		strcpy(rmsg->data.string_data,"No monitor available");

		printf("No monitor available, informing patient\n");

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
		printf("Handling connection request from monitor\n");

		//create shared memory to be used in hospital system -> monitor un-prompted communication
		create_shared_memory(HS_MONITOR_SHMEM_SIZE, monitor_pid, &shmem_ptr, &monitor_handle);

		//write default value to patient name section
		write_shmem(shmem_ptr, "No patient\0", HS_SHMEM_OFFSET_PATIENT_NAME, 20);

		//write default status to monitor section
		write_shmem(shmem_ptr, "Running\0", HS_SHMEM_OFFSET_STATUS, 10);

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
	case HS_MSG_PATIENT_CRITICAL:


		printf("Received notification of patient in critical condition from monitor %d, status code %d\n",msg->id, msg->statusCode);

		//todo: add actual handling here?


		//notify the monitor that help is on the way
		rmsg->messageReplyType = HS_REPLY_HELP;
		strcpy(rmsg->data.string_data,"Help is on the way");

		break;

	default:
		break;
	}
}

void handleShutdown(name_attach_t** channel, monitor_array_t* monitorList){

	printf("Shutting down hospital system\n");

	for(int i = 0; i < monitorList->length; i++){
		void* monitor_shmem_ptr = monitorList->monitors[i].shmem_ptr;

		printf("Shutting down monitor %d\n", monitorList->monitors[i].id);

		write_shmem(monitor_shmem_ptr, "Shutdown\0", HS_SHMEM_OFFSET_STATUS, 10);
	}

	printf("Waiting to ensure monitors shut down...");

	sleep(5);

	//clear shared memory allocations
	for(int i = 0; i < monitorList->length; i++){
		void* monitor_shmem_ptr = monitorList->monitors[i].shmem_ptr;

		munmap(monitor_shmem_ptr, 4096);
	}


	//clear allocated memory and free channel
	name_detach(*channel, 0);
	free(monitorList->monitors);

	printf("Shutdown complete!");

	exit(0);
}

char* generatePatientServerName(){

	char* serverName = malloc(17);

	time_t t;
	srand((unsigned)time(&t));

	memcpy(serverName, PATIENT_SERVER_NAME_HEADER, strlen(PATIENT_SERVER_NAME_HEADER));

	//append random lowercase characters to finish the random server name
	for(int i = strlen(PATIENT_SERVER_NAME_HEADER); i < 16; i++){
		serverName[i] = (char)rand()%26 + 97;
	}

	serverName[16] = '\0';

	return serverName;
}


