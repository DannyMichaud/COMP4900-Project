#include <stdint.h>
#include <sys/iomsg.h>
#include <sys/mman.h>

#define PATIENT_SHMEM_SIZE 4096

#define HS_MONITOR_SHMEM_SIZE 4096

//for shared memory offsets on monitored values
#define PATIENT_SHMEM_OFFSET_HEARTBEAT 100
#define PATIENT_SHMEM_OFFSET_BLOOD_PRESSURE_SYSTOLIC 110
#define PATIENT_SHMEM_OFFSET_BLOOD_PRESSURE_DIASTOLIC 115
#define PATIENT_SHMEM_OFFSET_TEMPERATURE 120
#define PATIENT_SHMEM_OFFSET_RESPIRATION 130
#define PATIENT_SHMEM_OFFSET_OXYGEN_SATURATION 140

//for shared memory offsets on monitor's fluid levels
#define PATIENT_SHMEM_OFFSET_TREATMENT_IV_FLUID 300
#define PATIENT_SHMEM_OFFSET_TREATMENT_MEDICINE 310


#define PATIENT_HEARTBEAT_LOWER_LIMIT 60
#define PATIENT_HEARTBEAT_UPPER_LIMIT 100
#define PATIENT_SYSTOLIC_BP_LOWER_LIMIT 100
#define PATIENT_SYSTOLIC_BP_UPPER_LIMIT 130
#define PATIENT_DIASTOLIC_BP_LOWER_LIMIT 60
#define PATIENT_DIASTOLIC_BP_UPPER_LIMIT 80
#define PATIENT_TEMPERATURE_LOWER_LIMIT 36.1
#define PATIENT_TEMPERATURE_UPPER_LIMIT 37.2
#define PATIENT_RESPIRATION_LOWER_LIMIT 12
#define PATIENT_RESPIRATION_UPPER_LIMIT 16
#define PATIENT_OXYGEN_SATURATION_LOWER_LIMIT 95

#define GET_SHMEM_MSG_TYPE (_IO_MAX+200)

//for shared memory between monitor and hospital system
#define HS_SHMEM_OFFSET_PATIENT_NAME 100

// ask for a shared_mem_bytes sized object
typedef struct get_shmem_msg {
	uint16_t type;
	unsigned shared_mem_bytes;
} get_shmem_msg_t;

// response provides a handle to that object
typedef struct get_shmem_resp {
	shm_handle_t mem_handle;
} get_shmem_resp_t;


int create_shared_memory(unsigned nbytes, int client_pid, void **ptr, shm_handle_t *handle);

void write_shmem_int(void* shmem_ptr, int int_val, uint16_t offset, uint8_t range);

void write_shmem_float(void* shmem_ptr, float float_val, uint16_t offset, uint8_t range);

void write_shmem(void* shmem_ptr, char* output, uint16_t offset, uint8_t range);

char* read_shmem(void* shmem_ptr, uint16_t offset);

int read_shmem_int(void* shmem_ptr, uint16_t offset);

float read_shmem_float(void* shmem_ptr, uint16_t offset);

