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
#include "../hospital_system/hospital_system_address.h"
#include "../hospital_system/hospital_system_messages.h"



typedef union
{
	uint16_t type;
	struct _pulse pulse;
	get_shmem_msg_t get_shmem;
} recv_buf_t;

int main(int argc, char **argv) {

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

	//close connection to hospital system as it is no longer necessary
	name_close(coid);

	printf("Starting patient with name %s\n", msgReply.data.string_data);

	startPatientServer(msgReply.data.string_data);
}

void startPatientServer(char* patientServerName){
	// register our name for a channel

	name_attach_t* patientChannel = name_attach(NULL, patientServerName, 0);

	while(1){
		recv_buf_t msg;

		struct _msg_info info;

		int rcvid = MsgReceive(patientChannel->chid, &msg, sizeof(msg), &info);

		printf("Received message from monitor\n");

		pid_t pid = info.pid;

		if (rcvid == 0) {
			//received a pulse

		} else {
			handleMessageFromMonitor(msg, pid, rcvid);
		}

	}

	name_detach(patientChannel, 0);
	free(patientChannel);
}


void handleMessageFromMonitor(recv_buf_t msg, pid_t pid, int rcvid){
	switch(msg.type){
	case GET_SHMEM_MSG_TYPE: ;

		get_shmem_resp_t rmsg;
		void* ptr;

		create_shared_memory(msg.get_shmem.shared_mem_bytes, pid, &ptr, &rmsg.mem_handle);

		//reply to client with the handle
		int result = MsgReply(rcvid, EOK, &rmsg, sizeof(get_shmem_resp_t));

		//todo - set up threads to update shared memory
		pthread_t threads[5];

		initVitalThreads(threads, ptr);

		break;
	default:
		break;
	}
}

void initVitalThreads(pthread_t* threads, void* ptr){

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

	//create thread for each vital
	pthread_create(&threads[0], NULL,(void*) &getVitalOnInterval, heartbeatThread);
	pthread_create(&threads[1], NULL,(void*) &getVitalOnInterval, systolicBPThread);
	pthread_create(&threads[2], NULL,(void*) &getVitalOnInterval, diastolicBPThread);
	pthread_create(&threads[3], NULL,(void*) &getVitalOnInterval, temperatureThread);
	pthread_create(&threads[4], NULL,(void*) &getVitalOnInterval, respirationThread);
	pthread_create(&threads[5], NULL,(void*) &getVitalOnInterval, oxygenSaturationThread);
}


// Will create timer, each interval will get value for corresponding vital, and write to corresponding shmem location...
void getVitalOnInterval(patient_vital_thrinfo_t* vitalInfo){
	while(1){
		vital_val_t vitalValue = getVital(vitalInfo->vitalType);

		if(vitalInfo->vitalType == TEMPERATURE){
			write_shmem_float(vitalInfo->shmem_ptr, vitalValue.float_val, vitalInfo->offset, vitalInfo->range);
			printf("Temperature: %s\n", (char *) vitalInfo->shmem_ptr + vitalInfo->offset);
		}
		else {
			write_shmem_int(vitalInfo->shmem_ptr, vitalValue.int_val, vitalInfo->offset, vitalInfo->range);
			printf("Vital: %s\n", (char *) vitalInfo->shmem_ptr + vitalInfo->offset);
		}

		sleep(1);
	}
}

void updateVitalOnInterval(patient_vital_thrinfo_t* vitalInfo) {
	while(1) {

	}
}

