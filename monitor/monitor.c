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
#include "../shared_memory/shared_memory.h"
#include "../hospital_system/hospital_system_address.h"
#include "../hospital_system/hospital_system_messages.h"

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

	printf("Id: %d\n", msgReply.data.int_data);

	//create thread to monitor shared memory
	pthread_t thread;

	pthread_create(&thread, NULL,(void*) &monitorHospitalSystemSharedMemory, msgReply.data2.shmem_handle);

	//temporary until proper client loop set up
	while(1){
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

	sleep(3);

	// Monitor patient vitals
	monitorPatientVitals(patient_shmem_ptr);

	munmap(patient_shmem_ptr, 4096);
}

void monitorPatientVitals(void* shmem_ptr) {

	// Get vitals
	int heartrate, systolicBP, diastolicBP, respiration, oxygenSaturation;
	float temperature;

	while(1) {
		heartrate = read_shmem_int(shmem_ptr, PATIENT_SHMEM_OFFSET_HEARTBEAT);
		systolicBP = read_shmem_int(shmem_ptr, PATIENT_SHMEM_OFFSET_BLOOD_PRESSURE_SYSTOLIC);
		diastolicBP = read_shmem_int(shmem_ptr, PATIENT_SHMEM_OFFSET_BLOOD_PRESSURE_DIASTOLIC);
		temperature = read_shmem_float(shmem_ptr, PATIENT_SHMEM_OFFSET_TEMPERATURE);
		respiration = read_shmem_int(shmem_ptr, PATIENT_SHMEM_OFFSET_RESPIRATION);
		oxygenSaturation = read_shmem_int(shmem_ptr, PATIENT_SHMEM_OFFSET_OXYGEN_SATURATION);

		// Check patient heart rate
		if (heartrate < PATIENT_HEARTBEAT_LOWER_LIMIT || heartrate > PATIENT_HEARTBEAT_UPPER_LIMIT) {
			// Send message to hospital system (critical)
			printf("(TEMP) Heartrate critical: %d\n", heartrate);
		}

		// Check patient systolic blood pressure
		if (systolicBP < PATIENT_SYSTOLIC_BP_LOWER_LIMIT || systolicBP > PATIENT_SYSTOLIC_BP_UPPER_LIMIT) {
			// Send message to hospital system (critical)
			printf("(TEMP) Systolic BP critical: %d\n", systolicBP);
		}

		// Check patient diastolic blood pressure
		if (diastolicBP < PATIENT_DIASTOLIC_BP_LOWER_LIMIT || diastolicBP > PATIENT_DIASTOLIC_BP_UPPER_LIMIT) {
			// Send message to hospital system (critical)
			printf("(TEMP) Diastolic BP critical: %d\n", diastolicBP);
		}

		// Check patient temperature
		if (temperature < PATIENT_TEMPERATURE_LOWER_LIMIT || temperature > PATIENT_TEMPERATURE_UPPER_LIMIT) {
			// Send message to hospital system (critical)
			printf("(TEMP) Temperature critical: %.1f\n", temperature);
		}

		// Check patient respiration
		if (respiration < PATIENT_RESPIRATION_LOWER_LIMIT || respiration > PATIENT_RESPIRATION_UPPER_LIMIT) {
			// Send message to hospital system (critical)
			printf("(TEMP) Respiration critical: %d\n", respiration);
		}

		// Check patient oxygen saturation
		if (oxygenSaturation > PATIENT_OXYGEN_SATURATION_LOWER_LIMIT) {
			// Send message to hospital system (critical)
			printf("(TEMP) Respiration critical: %d\n", respiration);
		}
		sleep(1);
	}
}
