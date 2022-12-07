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
#include "patient_vitals.h"
#include "patient_vitals_utils.h"
#include "../hospital_system/hospital_system_address.h"
#include "../hospital_system/hospital_system_messages.h"

typedef union
{
	uint16_t type;
	struct _pulse pulse;
	get_shmem_msg_t get_shmem;
} recv_buf_t;

patientHealth_t patientHealth;
illness_t illness;

// argv[1] = vital type
// argv[2] = severity
int main(int argc, char **argv) {
	// set patient illness based on args
	if (argc > 2) {
		illness.vitalType = atoi(argv[1]);
		illness.severity = atoi(argv[2]);
	} else {
		printf("ERROR: Please input valid arguments.\n");
		exit(-1);
	}
	patientHealth = HEALTHY;
	//todo: request hospital server for server name

	getServerNameFromHospitalSystem();

	return 0;
}

void getServerNameFromHospitalSystem(){

	/* find our server to get a coid*/
	int coid = name_open(HOSPITAL_SERVER_NAME, 0);

	//get our server name from the hospital system
	hospital_system_msg_to_t msg = {SOURCE_PATIENT, HS_MSG_CONNECT};
	hospital_system_msg_from_t msgReply;

	int status = MsgSend(coid, &msg, sizeof(hospital_system_msg_to_t), &msgReply, sizeof(hospital_system_msg_from_t));

	if(status == -1){
		printf("Error: No hospital system\n");
		exit(1);
	}

	//close connection to hospital system as it is no longer necessary
	name_close(coid);

	printf("Starting patient with name %s\n", msgReply.data.string_data);

	startPatientServer(msgReply.data.string_data);
}

void startPatientServer(char* patientServerName){
	// register our name for a channel

	name_attach_t* patientChannel = name_attach(NULL, patientServerName, 0);
	recv_buf_t msg;
	struct _msg_info info;

	while(1){
		recv_buf_t msg;

		struct _msg_info info;

		int rcvid = MsgReceive(patientChannel->chid, &msg, sizeof(msg), &info);

		printf("(PATIENT) Received message from monitor with rcvid %d\n", rcvid);

		pid_t pid = info.pid;

		handleMessageFromMonitor(msg, pid, rcvid);
	}

	name_detach(patientChannel, 0);
	free(patientChannel);
}


void handleMessageFromMonitor(recv_buf_t msg, pid_t pid, int rcvid){

	void* ptr;

	//handle monitor shutting down

	if(rcvid == -1 || (rcvid == 0 && msg.pulse.code == _PULSE_CODE_DISCONNECT)){
		printf("(PATIENT) Monitor has shut down, shutting down patient");

		if(ptr != NULL){
			munmap(ptr, 4096);
		}
		exit(0);
	}
	else if(rcvid == 0){
		return;
	}

	printf("message type: %d\n", msg.type);



	switch(msg.type){
	case GET_SHMEM_MSG_TYPE: ;

		get_shmem_resp_t rmsg;

		printf("inside handleMessageFromMonitor case\n");

		create_shared_memory(msg.get_shmem.shared_mem_bytes, pid, &ptr, &rmsg.mem_handle);

		//reply to client with the handle
		int result = MsgReply(rcvid, EOK, &rmsg, sizeof(get_shmem_resp_t));

		//todo - set up threads to update shared memory
		pthread_t threads[7];

		initVitalThreads(threads, ptr);

		break;
	default:
		break;
	}
}

void initVitalThreads(pthread_t* threads, void* ptr){
	// Initialize vitals with default values for now
	initVitals(NULL);

	// Allocate memory for vital threads
	patient_vital_thrinfo_t* heartbeatThread = malloc(sizeof(patient_vital_thrinfo_t));
	*heartbeatThread = (patient_vital_thrinfo_t){HEARTBEAT, PATIENT_SHMEM_OFFSET_HEARTBEAT, 10, ptr};
	patient_vital_thrinfo_t* systolicBPThread = malloc(sizeof(patient_vital_thrinfo_t));
	*systolicBPThread = (patient_vital_thrinfo_t){BLOOD_PRESSURE_SYSTOLIC, PATIENT_SHMEM_OFFSET_BLOOD_PRESSURE_SYSTOLIC, 5, ptr};
	patient_vital_thrinfo_t* diastolicBPThread = malloc(sizeof(patient_vital_thrinfo_t));
	*diastolicBPThread = (patient_vital_thrinfo_t){BLOOD_PRESSURE_DIASTOLIC, PATIENT_SHMEM_OFFSET_BLOOD_PRESSURE_DIASTOLIC, 5, ptr};
	patient_vital_thrinfo_t* temperatureThread = malloc(sizeof(patient_vital_thrinfo_t));
	*temperatureThread = (patient_vital_thrinfo_t){TEMPERATURE, PATIENT_SHMEM_OFFSET_TEMPERATURE, 10, ptr};
	patient_vital_thrinfo_t* respirationThread = malloc(sizeof(patient_vital_thrinfo_t));
	*respirationThread = (patient_vital_thrinfo_t){RESPIRATION, PATIENT_SHMEM_OFFSET_RESPIRATION, 10, ptr};
	patient_vital_thrinfo_t* oxygenSaturationThread = malloc(sizeof(patient_vital_thrinfo_t));
	*oxygenSaturationThread = (patient_vital_thrinfo_t){OXYGEN_SATURATION, PATIENT_SHMEM_OFFSET_OXYGEN_SATURATION, 10, ptr};

	printf("Creating threads\n");

	//create thread for each vital (to write to shared memory)
	pthread_create(&threads[0], NULL,(void*) &getVitalOnInterval, heartbeatThread);
	pthread_create(&threads[1], NULL,(void*) &getVitalOnInterval, systolicBPThread);
	pthread_create(&threads[2], NULL,(void*) &getVitalOnInterval, diastolicBPThread);
	pthread_create(&threads[3], NULL,(void*) &getVitalOnInterval, temperatureThread);
	pthread_create(&threads[4], NULL,(void*) &getVitalOnInterval, respirationThread);
	pthread_create(&threads[5], NULL,(void*) &getVitalOnInterval, oxygenSaturationThread);

	printf("Creating update threads\n");
	//HARD-CODED FOR NOW, PASS IN SOMEWHERE
	pthread_create(&threads[6], NULL, (void*) &updateVitalOnInterval, NULL);
}


// Will create timer, each interval will get value for corresponding vital, and write to corresponding shmem location...
void getVitalOnInterval(patient_vital_thrinfo_t* vitalInfo){
	while(1){
		vital_val_t vitalValue = getVital(vitalInfo->vitalType);

		if(vitalInfo->vitalType == TEMPERATURE){
			write_shmem_float(vitalInfo->shmem_ptr, vitalValue.float_val, vitalInfo->offset, vitalInfo->range);
			printf("(PATIENT) Temperature: %s\n", (char *) vitalInfo->shmem_ptr + vitalInfo->offset);
		}
		else {
			write_shmem_int(vitalInfo->shmem_ptr, vitalValue.int_val, vitalInfo->offset, vitalInfo->range);
			printf("(PATIENT) Vital: %s\n", (char *) vitalInfo->shmem_ptr + vitalInfo->offset);
		}

		sleep(3);
	}
}

// Update vitals randomly based on patient illness
void updateVitalOnInterval() {
	// Update vital on condition of them being healthy
	// May need changes here
	while(isPatientHealthy(illness.vitalType)) {
		updateVital(illness.vitalType, generateVitalValue(illness));

		sleep(2);
	}

	return;
}

