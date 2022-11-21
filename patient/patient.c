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

int main(int argc, char **argv) {

	//todo: request hospital server for server name

	startPatientServer("/patient-name-temp");

	return 0;
}

void startPatientServer(char* patientServerName){
	// register our name for a channel
	name_attach_t* patientChannel = name_attach(NULL, patientServerName, 0);

	while(1){

	}

	name_detach(patientChannel, 0);
	free(patientChannel);
}
