/*
 * monitor.c
 *
 * Used to monitor patient vitals, and report to hospital system if things are going wrong...
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
#include "monitor.h"
#include "monitor_treatments.h"
#include "monitor_internal_messaging.h"
#include "../shared_memory/shared_memory.h"
#include "../hospital_system/hospital_system_address.h"
#include "../hospital_system/hospital_system_messages.h"

int monitorId = 0;
int doctorPresent = 0;

int main(int argc, char **argv) {
	//start by connecting to hospital system to get registered
	connectToHospitalSystem();

	return 0;
}

void connectToHospitalSystem(){
	// find our server to get a coid
	int coid = name_open(HOSPITAL_SERVER_NAME, 0);

	hospital_system_msg_to_t msg = {SOURCE_MONITOR, HS_MSG_CONNECT};
	hospital_system_msg_from_t msgReply;

	int status = MsgSend(coid, &msg, sizeof(hospital_system_msg_to_t), &msgReply, sizeof(hospital_system_msg_from_t));

	monitorId = msgReply.data.int_data;

	printf("Connected to hospital system and got id: %d\n", msgReply.data.int_data);

	int intChannelNameLength = snprintf(NULL, 0, "monitor-int-%d", monitorId);

	char* internalChannelName = malloc(sizeof(intChannelNameLength+1));

	snprintf(internalChannelName, intChannelNameLength+1, "monitor-int-%d", monitorId);

	name_attach_t* internalChannel = name_attach(NULL, internalChannelName, 0);

	printf("Set up internal channel at: %s\n", internalChannelName);

	free(internalChannelName);

	//create thread to monitor shared memory
	pthread_t thread;

	pthread_create(&thread, NULL,(void*) &monitorHospitalSystemSharedMemory, msgReply.data2.shmem_handle);

	//temporary until proper client loop set up
	while(1){

		internal_monitor_msg_t int_msg;
		internal_monitor_msg_t int_rmsg = { 0 };

		int rcvid = MsgReceive(internalChannel->chid, &int_msg, sizeof(int_msg), NULL);

		if(rcvid > 0){
			printf("Received critical status code %d\n", int_msg.status);
		}

		//send empty reply to monitoring thread to unblock it
		int status = MsgReply(rcvid, EOK, &int_rmsg, sizeof(int_rmsg));

		if(status == -1){
			printf("Internal channel broke :( \n");
			return;
		}

		//send message to hospital system informing it of patient's status
		hospital_system_msg_to_t msg = {SOURCE_MONITOR, HS_MSG_PATIENT_CRITICAL, monitorId, int_msg.status};
		hospital_system_msg_from_t msgReply;

		status = MsgSend(coid, &msg, sizeof(hospital_system_msg_to_t), &msgReply, sizeof(hospital_system_msg_from_t));

		if(status == -1){
			printf("Hospital system broke/has been shut off :(\n");
			return;
		}

		//wait for reply
		if(msgReply.messageReplyType == HS_REPLY_HELP){
			printf("Hospital system sent help, message '%s'\n", msgReply.data.string_data);

			// Alert hospital system
			doctorPresent = 1;
			msg.messageType = HS_MSG_DOCTOR_PRESENT;
			status = MsgSend(coid, &msg, sizeof(hospital_system_msg_to_t), &msgReply, sizeof(hospital_system_msg_from_t));

			if (status == -1) {
				printf("Internal channel broke :( \n");
				return;
			}
		}

		sleep(1);
	}
}

void monitorHospitalSystemSharedMemory(shm_handle_t shmem_handle){
    //open handle to get fd
	int fd = shm_open_handle(shmem_handle, O_RDWR);

    //map the shared memory
	void *monitor_shmem_ptr = mmap(NULL, HS_MONITOR_SHMEM_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

	//once mapped, we don't need the fd anymore
	close(fd);

	printf("Started monitoring shared memory with hospital system\n");

	int waitingForPatient = 1;

	while(1){
		if(waitingForPatient){
			char* message = read_shmem(monitor_shmem_ptr, HS_SHMEM_OFFSET_PATIENT_NAME);

			printf("(SHARED MEMORY MONITORING) Read patient name from server: %s\n", message);

			//if server name is ready, connect on new thread and end this one
			if(strcmp(message, "No patient") != 0){
				pthread_t thread;

				printf("creating connectopatient thread\n");

				pthread_create(&thread, NULL,(void*) &connectToPatient, message);

				waitingForPatient = 0;
			}
			free(message);
		}

		//check if hospital system is still running, terminate monitor otherwise
		char* status = read_shmem(monitor_shmem_ptr, HS_SHMEM_OFFSET_STATUS);

		if(strcmp(status, "Running") != 0){
			printf("Hospital system has shut down, closing monitor\n");

			free(status);
			exit(0);
		}

		free(status);

		sleep(1);
	}
}

void connectToPatient(char* patientServerName){

	/* find our server to get a coid*/
	int coid = -1;

	printf("Trying to connect to patient at %s\n", patientServerName);
	//may have to try a few times while things get set up
	while(coid == -1){
		coid = name_open(patientServerName, 0);
		sleep(1);
	}

	printf("Successfully connected to patient at %s, coid %d\n", patientServerName, coid);

	int intChannelNameLength = snprintf(NULL, 0, "monitor-int-%d", monitorId);

	char* internalChannelName = malloc(sizeof(intChannelNameLength+1));

	snprintf(internalChannelName, intChannelNameLength+1, "monitor-int-%d", monitorId);

	int int_coid = name_open(internalChannelName, 0);

	free(internalChannelName);

	printf("Internal channel successfully linked, int_coid %d\n", int_coid);

	//request for shared memory
	get_shmem_msg_t get_msg;
	get_shmem_resp_t get_msg_reply;

	get_msg.type = GET_SHMEM_MSG_TYPE;
	get_msg.shared_mem_bytes = PATIENT_SHMEM_SIZE;

	printf("Sending connection request to patient\n");

	//send the message to the server and get a reply
	int status = MsgSend(coid, &get_msg, sizeof(get_shmem_msg_t), &get_msg_reply, sizeof(get_shmem_resp_t));

    //open handle to get fd
	int fd = shm_open_handle(get_msg_reply.mem_handle, O_RDWR);

    //map the shared memory
	void *patient_shmem_ptr = mmap(NULL, PATIENT_SHMEM_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

	//once mapped, we don't need the fd anymore
	close(fd);

	printf("Response code for get message: %d\n", status);

	//init treatments
	initTreatments(patient_shmem_ptr);

	sleep(3);

	// Monitor patient vitals
	monitorPatientVitals(patient_shmem_ptr, int_coid);

	munmap(patient_shmem_ptr, 4096);
}

void monitorPatientVitals(void* shmem_ptr, int int_coid) {

	// Get vitals
	patient_vitals_t patientVitals;

	while(1) {
		patientVitals.heartRate = read_shmem_int(shmem_ptr, PATIENT_SHMEM_OFFSET_HEARTBEAT);
		patientVitals.systolicBP = read_shmem_int(shmem_ptr, PATIENT_SHMEM_OFFSET_BLOOD_PRESSURE_SYSTOLIC);
		patientVitals.diastolicBP = read_shmem_int(shmem_ptr, PATIENT_SHMEM_OFFSET_BLOOD_PRESSURE_DIASTOLIC);
		patientVitals.temperature = read_shmem_float(shmem_ptr, PATIENT_SHMEM_OFFSET_TEMPERATURE);
		patientVitals.respiration = read_shmem_int(shmem_ptr, PATIENT_SHMEM_OFFSET_RESPIRATION);
		patientVitals.oxygenSaturation = read_shmem_int(shmem_ptr, PATIENT_SHMEM_OFFSET_OXYGEN_SATURATION);

		//track patient statuses
		int status = 0;

		// Check patient heart rate
		if (patientVitals.heartRate < PATIENT_HEARTBEAT_LOWER_LIMIT || patientVitals.heartRate > PATIENT_HEARTBEAT_UPPER_LIMIT) {
			status |= 1 << 0;
		}

		// Check patient systolic blood pressure
		if (patientVitals.systolicBP < PATIENT_SYSTOLIC_BP_LOWER_LIMIT || patientVitals.systolicBP > PATIENT_SYSTOLIC_BP_UPPER_LIMIT) {
			status |= 1 << 1;
		}

		// Check patient diastolic blood pressure
		if (patientVitals.diastolicBP < PATIENT_DIASTOLIC_BP_LOWER_LIMIT || patientVitals.diastolicBP > PATIENT_DIASTOLIC_BP_UPPER_LIMIT) {
			status |= 1 << 2;
		}

		// Check patient temperature
		if (patientVitals.temperature < PATIENT_TEMPERATURE_LOWER_LIMIT || patientVitals.temperature > PATIENT_TEMPERATURE_UPPER_LIMIT) {
			status |= 1 << 3;
		}

		// Check patient respiration
		if (patientVitals.respiration < PATIENT_RESPIRATION_LOWER_LIMIT || patientVitals.respiration > PATIENT_RESPIRATION_UPPER_LIMIT) {
			status |= 1 << 4;
		}

		// Check patient oxygen saturation
		if (patientVitals.oxygenSaturation > PATIENT_OXYGEN_SATURATION_LOWER_LIMIT) {
			status |= 1 << 5;
		}

		printf("Status: %d\n", status);

		//if any patient vitals critical, send message over internal channel
		if(status != 0 && !doctorPresent){
			internal_monitor_msg_t msg = { status };
			internal_monitor_msg_t msgReply;
			printf("Critical status to internally send: %d\n", msg.status);
			int reply = MsgSend(int_coid, &msg, sizeof(hospital_system_msg_to_t), &msgReply, sizeof(hospital_system_msg_from_t));
		}

		sleep(1);
	}
}
