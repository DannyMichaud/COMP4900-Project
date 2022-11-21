/*
 * monitor.c
 *
 * Used to monitor patient vitals, and report to hospital system if things are going wrong...
 */

#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>     // #define for ND_LOCAL_NODE is in here
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include "monitor.h"

int main(int argc, char **argv) {

	//todo: receive server name from hospital server

	connectToPatient("/patient-name-temp");

	return 0;
}

void connectToPatient(char* patientServerName){

	/* find our server to get a coid*/
	int coid = name_open(patientServerName, 0);

}
