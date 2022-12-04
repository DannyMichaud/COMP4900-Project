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


// implicit declarations
void initVitals(patient_vitals_t* initialVitals);
vital_val_t getVital(vitalType_t vitalType);
void updateVital(vitalType_t vitalType, vital_val_t newValue);
vital_val_t generateVitalValue(illness_t illness);
float generateFloatInRange(float lower, float upper);
int generateNumberInRange(int lower, int upper);
int isPatientHealthy(vitalType_t vitalType);

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

vital_val_t generateVitalValue(illness_t illness) {
	int lower, upper;
	float floatLower, floatUpper;
	vital_val_t newValue;

	switch(illness.vitalType) {
	case HEARTBEAT:
		lower = PATIENT_HEARTBEAT_LOWER_LIMIT - illness.severity;
		upper = PATIENT_HEARTBEAT_UPPER_LIMIT - illness.severity;
		break;
	case BLOOD_PRESSURE_SYSTOLIC:
		lower = PATIENT_SYSTOLIC_BP_LOWER_LIMIT - illness.severity;
		upper = PATIENT_SYSTOLIC_BP_UPPER_LIMIT - illness.severity;
		break;
	case BLOOD_PRESSURE_DIASTOLIC:
		lower = PATIENT_DIASTOLIC_BP_LOWER_LIMIT - illness.severity;
		upper = PATIENT_DIASTOLIC_BP_UPPER_LIMIT - illness.severity;
		break;
	case TEMPERATURE:
		floatLower = PATIENT_TEMPERATURE_LOWER_LIMIT - illness.severity / 10;
		floatUpper = PATIENT_TEMPERATURE_UPPER_LIMIT - illness.severity / 10;
		break;
	case RESPIRATION:
		lower = PATIENT_RESPIRATION_LOWER_LIMIT - illness.severity;
		upper = PATIENT_RESPIRATION_UPPER_LIMIT - illness.severity;
		break;
	case OXYGEN_SATURATION:
		lower = PATIENT_OXYGEN_SATURATION_LOWER_LIMIT - illness.severity;
		upper = PATIENT_OXYGEN_SATURATION_LOWER_LIMIT + 30 - illness.severity;
		break;
	default:
		break;
	}

	if (illness.vitalType == TEMPERATURE) {
		newValue.float_val = generateFloatInRange(floatLower, floatUpper);
	} else {
		newValue.int_val = generateNumberInRange(lower, upper);
	}
}

float generateFloatInRange(float lower, float upper) {
	return generateNumberInRange(lower * 10, upper * 10) / 10.0;
}
// Generate a random number in the range [lower, upper]
int generateNumberInRange(int lower, int upper) {
	time_t t;
	srand((unsigned) time(&t));
	int newValue = (rand() % (upper - lower + 1)) + lower;
	printf("New value: %d\n", newValue);

	return newValue;
}

int isPatientHealthy(vitalType_t vitalType) {
	switch(vitalType) {
	case HEARTBEAT:
		return patientVitals.heartRate >= PATIENT_HEARTBEAT_LOWER_LIMIT &&
				patientVitals.heartRate <= PATIENT_HEARTBEAT_UPPER_LIMIT;
		break;
	case BLOOD_PRESSURE_SYSTOLIC:
		return patientVitals.systolicBP >= PATIENT_SYSTOLIC_BP_LOWER_LIMIT &&
				patientVitals.systolicBP <= PATIENT_SYSTOLIC_BP_UPPER_LIMIT;
		break;
	case BLOOD_PRESSURE_DIASTOLIC:
		return patientVitals.diastolicBP >= PATIENT_DIASTOLIC_BP_LOWER_LIMIT &&
				patientVitals.diastolicBP <= PATIENT_DIASTOLIC_BP_UPPER_LIMIT;
		break;
	case TEMPERATURE:
		return patientVitals.temperature >= PATIENT_TEMPERATURE_LOWER_LIMIT &&
				patientVitals.temperature <= PATIENT_TEMPERATURE_UPPER_LIMIT;
		break;
	case RESPIRATION:
		return patientVitals.respiration >= PATIENT_RESPIRATION_LOWER_LIMIT &&
				patientVitals.respiration <= PATIENT_RESPIRATION_UPPER_LIMIT;
		break;
	case OXYGEN_SATURATION:
		return patientVitals.oxygenSaturation >= PATIENT_OXYGEN_SATURATION_LOWER_LIMIT;
		break;
	default:
		return -1;
		break;
	}
}
