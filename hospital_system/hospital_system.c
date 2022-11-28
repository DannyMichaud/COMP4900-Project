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
#include "hospital_system_data.h"
#include "hospital_system_messages.h"


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

	monitor_array_t monitorList;

	while(1){

		hospital_system_msg_to_t msg;

		int rcvid = MsgReceive((*channel)->chid, &msg, sizeof(msg), NULL);

		hospital_system_msg_from_t rmsg;

		//handle message depending on whether it came from patient or monitor
		switch(msg.messageSender){
		case SOURCE_PATIENT:
			handlePatientMessage(&msg, &rmsg, &monitorList);
			break;
		case SOURCE_MONITOR:
			handleMonitorMessage(&msg, &rmsg, &monitorList);
			break;
		default:
			break;
		}

		//send message back to client
		int result = MsgReply(rcvid, EOK, &rmsg, sizeof(rmsg));

		//todo: extra handling depending on message sent back?
	}

	free(monitorList);
}

void handlePatientMessage(hospital_system_msg_to_t* msg, hospital_system_msg_from_t* rmsg, monitor_array_t* monitorList){

}

void handleMonitorMessage(hospital_system_msg_to_t* msg, hospital_system_msg_from_t* rmsg, monitor_array_t* monitorList){

	switch(msg->messageType){
	case HS_MSG_CONNECT:

		//track new monitor with id equal to previous length of array
		monitorList->monitors = realloc((monitorList->length + 1)*sizeof(monitor_data_t));
		*(monitorList->monitors[monitorList->length]) = (monitor_data_t){monitorList->length, 0};
		monitorList->length++;

		//send back id to monitor for future reference
		rmsg.messageReplyType = HS_REPLY_ID;
		rmsg.data.int_data = monitorList->monitors[monitorList->length]->id;

		break;
	default:
		break;
	}
}
