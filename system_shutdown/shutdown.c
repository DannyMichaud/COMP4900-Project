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
#include "../hospital_system/hospital_system.h"
#include "../hospital_system/hospital_system_address.h"


//for manually shutting down hospital system
int main(int argc, char **argv) {

	int coid = name_open(HOSPITAL_SERVER_NAME,0);

	printf("Shutting down hospital system...\n");

	hospital_system_msg_to_t shutdownMsg = { SOURCE_SHUTDOWN };

	hospital_system_msg_from_t reply;

	MsgSend(coid, &shutdownMsg, sizeof(hospital_system_msg_to_t), &reply, sizeof(hospital_system_msg_from_t));

	return 0;
}
