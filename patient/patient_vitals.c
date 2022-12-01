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
#include "patient_vitals.h"

patient_vitals_t patientVitals;

void initVitals(patient_vitals_t* initialVitals) {
	if (initialVitals != NULL) {
		patientVitals = *initialVitals;
	} else {
		patientVitals.heartRate = DEFAULT_HEARTRATE;
		patientVitals.systolicBP = DEFAULT_SYSTOLIC_BP;
		patientVitals.diastolicBP = DEFAULT_DIASTOLIC_BP;
		patientVitals.temperature = DEFAULT_TEMPERATURE;
		patientVitals.respiration = DEFAULT_RESPIRATION;
		patientVitals.oxygenSaturation = DEFAULT_OXYGEN_SATURATION;
	}
}

//todo: do something other than global variables for this....
vital_val_t getVital(vitalType_t vitalType){

	vital_val_t returnVal;

	switch(vitalType) {
	case HEARTBEAT:
		returnVal.int_val = patientVitals.heartRate;
		break;
	case BLOOD_PRESSURE_SYSTOLIC:
		returnVal.int_val = patientVitals.systolicBP;
		break;
	case BLOOD_PRESSURE_DIASTOLIC:
		returnVal.int_val = patientVitals.diastolicBP;
		break;
	case TEMPERATURE:
		returnVal.float_val = patientVitals.temperature;
		break;
	case RESPIRATION:
		returnVal.int_val = patientVitals.respiration;
		break;
	case OXYGEN_SATURATION:
		returnVal.int_val = patientVitals.oxygenSaturation;
		break;
	default:
		break;
	}

	return returnVal;
}

void updateVital(vitalType_t vitalType, vital_val_t newValue) {
	switch(vitalType){
	case HEARTBEAT:
		patientVitals.heartRate = newValue.int_val;
		break;
	case BLOOD_PRESSURE_SYSTOLIC:
		patientVitals.systolicBP = newValue.int_val;
		break;
	case BLOOD_PRESSURE_DIASTOLIC:
		patientVitals.diastolicBP = newValue.int_val;
		break;
	case TEMPERATURE:
		patientVitals.temperature = newValue.float_val;
		break;
	case RESPIRATION:
		patientVitals.respiration = newValue.int_val;
		break;
	case OXYGEN_SATURATION:
		patientVitals.oxygenSaturation = newValue.int_val;
		break;
	default:
		break;
	}
}
