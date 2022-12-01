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

//possible todo: deal with this global var lolz
int monitorId = 0;

int main(int argc, char **argv) {

	//start by connecting to hospital system to get registered
	connectToHospitalSystem();

	return 0;
}

void connectToHospitalSystem(){
	/* find our server to get a coid*/
	int coid = name_open(HOSPITAL_SERVER_NAME, 0);

	hospital_system_msg_to_t msg = {SOURCE_MONITOR, HS_MSG_CONNECT};
	hospital_system_msg_from_t msgReply;

	int status = MsgSend(coid, &msg, sizeof(hospital_system_msg_to_t), &msgReply, sizeof(hospital_system_msg_from_t));

	monitorId = msgReply.data.int_data;

	printf("Id: %d\n", msgReply.data.int_data);

	//todo<internal channel>: create internal channel to communicate between monitor <-> hospital system, and patient <-> monitor threads
	//recommended: use server name 'monitor-int-<id>' (Ex: monitor-int-1)
	// (using channelcreate would require mechanism to store chid or global variable (both kinda suck)

	name_attach_t* internalChannel = name_attach(NULL, "monitor-int-" + monitorId, 0);

	printf("%s\n", "monitor-int-" + monitorId);

	//create thread to monitor shared memory
	pthread_t thread;

	pthread_create(&thread, NULL,(void*) &monitorHospitalSystemSharedMemory, msgReply.data2.shmem_handle);

	//temporary until proper client loop set up
	while(1){

		internal_monitor_msg_t msg;
		internal_monitor_msg_t rmsg = { 0 };

		//todo<internal channel>: wait to receive message from monitoring thread (patient <-> monitor)
		int rcvid = MsgReceive(internalChannel->chid, &msg, sizeof(msg), NULL);

		if(rcvid > 0){
			printf("Received critical status code %d\n", msg.status);
		}

		//send empty reply to monitoring thread to unblock it
		int status = MsgReply(rcvid, EOK, &rmsg, sizeof(rmsg));

		//send message to hospital system informing it of patient's status

		//wait for reply

		//handling of reply (if needed)

		sleep(1);
	}
}

void monitorHospitalSystemSharedMemory(shm_handle_t shmem_handle){

    //open handle to get fd
	int fd = shm_open_handle(shmem_handle, O_RDWR);

    //map the shared memory
	void *monitor_shmem_ptr = mmap(NULL, HS_MONITOR_SHMEM_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

	/* once mapped, we don't need the fd anymore */
	close(fd);

	while(1){
		char* message = read_shmem(monitor_shmem_ptr, HS_SHMEM_OFFSET_PATIENT_NAME);

		printf("Read patient name from server: %s\n", message);

		//if server name is ready, connect on new thread and end this one
		if(strcmp(message, "No patient") != 0){
			pthread_t thread;

			pthread_create(&thread, NULL,(void*) &connectToPatient, message);
			free(message);
			break;
		}

		free(message);

		sleep(1);
	}

}

void connectToPatient(char* patientServerName){

	/* find our server to get a coid*/
	int coid = -1;
	//may have to try a few times while things get set up
	while(coid == -1){
		coid = name_open(patientServerName, 0);
		sleep(1);
	}

	//todo<internal channel>: open the internal channel aswell (pass it into monitorPatientVitals function)
	//variable name: int_coid??
	int int_coid = name_open("monitor-int-" + monitorId, 0);

	/* request for shared memory */
	get_shmem_msg_t get_msg;
	get_shmem_resp_t get_msg_reply;

	get_msg.type = GET_SHMEM_MSG_TYPE;
	get_msg.shared_mem_bytes = PATIENT_SHMEM_SIZE;

	//send the message to the server and get a reply
	int status = MsgSend(coid, &get_msg, sizeof(get_shmem_msg_t), &get_msg_reply, sizeof(get_shmem_resp_t));

    //open handle to get fd
	int fd = shm_open_handle(get_msg_reply.mem_handle, O_RDWR);

    //map the shared memory
	void *patient_shmem_ptr = mmap(NULL, PATIENT_SHMEM_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

	/* once mapped, we don't need the fd anymore */
	close(fd);

	printf("Response code for get message: %d\n", status);

	//init treatments
	initTreatments(patient_shmem_ptr);

	sleep(3);

	// Monitor patient vitals
	monitorPatientVitals(patient_shmem_ptr, int_coid);

	munmap(patient_shmem_ptr, 4096);
}

//todo<internal channel>: include internal channel connection id (int_coid?) as parameter
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
			// Send message to hospital system (critical)
			status |= 1;
			//printf("(TEMP) Heartrate critical: %d\n", patientVitals.heartRate);
		}

		// Check patient systolic blood pressure
		if (patientVitals.systolicBP < PATIENT_SYSTOLIC_BP_LOWER_LIMIT || patientVitals.systolicBP > PATIENT_SYSTOLIC_BP_UPPER_LIMIT) {
			// Send message to hospital system (critical)
			status |= 1 << 1;
			//printf("(TEMP) Systolic BP critical: %d\n", patientVitals.systolicBP);
		}

		// Check patient diastolic blood pressure
		if (patientVitals.diastolicBP < PATIENT_DIASTOLIC_BP_LOWER_LIMIT || patientVitals.diastolicBP > PATIENT_DIASTOLIC_BP_UPPER_LIMIT) {
			// Send message to hospital system (critical)
			status |= 1 << 2;
			//printf("(TEMP) Diastolic BP critical: %d\n", patientVitals.diastolicBP);
		}

		// Check patient temperature
		if (patientVitals.temperature < PATIENT_TEMPERATURE_LOWER_LIMIT || patientVitals.temperature > PATIENT_TEMPERATURE_UPPER_LIMIT) {
			// Send message to hospital system (critical)
			status |= 1 << 3;
			//printf("(TEMP) Temperature critical: %.1f\n", patientVitals.temperature);
		}

		// Check patient respiration
		if (patientVitals.respiration < PATIENT_RESPIRATION_LOWER_LIMIT || patientVitals.respiration > PATIENT_RESPIRATION_UPPER_LIMIT) {
			// Send message to hospital system (critical)
			status |= 1 << 4;
			//printf("(TEMP) Respiration critical: %d\n", patientVitals.respiration);
		}

		// Check patient oxygen saturation
		if (patientVitals.oxygenSaturation > PATIENT_OXYGEN_SATURATION_LOWER_LIMIT) {
			// Send message to hospital system (critical)
			status |= 1 << 5;
			//printf("(TEMP) Respiration critical: %d\n", patientVitals.respiration);
		}

		//update treatments accordingly
		for(int i = IV_FLUID; i <= MEDICINE; i++){
			updateTreatment(shmem_ptr, i, &patientVitals);
		}

		//if any patient vitals critical, send message over internal channel
		if(status != 0){
			internal_monitor_msg_t msg = { status };
			internal_monitor_msg_t msgReply;
			printf("Critical status to internally send: %d\n", msg.status);
			int reply = MsgSend(int_coid, &msg, sizeof(hospital_system_msg_to_t), &msgReply, sizeof(hospital_system_msg_from_t));
		}


		sleep(1);
	}
}
