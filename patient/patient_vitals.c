/*
 * patient_vitals.c
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


int heartRate = 80;

int bloodPressureSystolic = 100;
int bloodPressureDiastolic = 70;

float temperature = 36.6;

int respiration = 14;

int oxygenSaturation = 95;



//todo: do something other than global variables for this....
vital_val_t getVital(vitalType_t vitalType){

	vital_val_t returnVal;

	switch(vitalType){
	case HEARTBEAT:
		returnVal.int_val = heartRate;
		break;
	case BLOOD_PRESSURE_SYSTOLIC:
		returnVal.int_val = bloodPressureSystolic;
		break;
	case BLOOD_PRESSURE_DIATOLIC:
		returnVal.int_val = bloodPressureDiastolic;
		break;
	case TEMPERATURE:
		returnVal.float_val = temperature;
		break;
	case RESPIRATION:
		returnVal.int_val = respiration;
		break;
	case OXYGEN_SATURATION:
		returnVal.int_val = oxygenSaturation;
		break;

	}

	return returnVal;
}
