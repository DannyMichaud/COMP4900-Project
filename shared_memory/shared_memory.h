#include <stdint.h>
#include <sys/iomsg.h>
#include <sys/mman.h>

#define PATIENT_SHMEM_SIZE 4096

//for shared memory offsets on monitored values
#define PATIENT_SHMEM_OFFSET_HEARTBEAT 100
#define PATIENT_SHMEM_OFFSET_BLOOD_PRESSURE_SYSTOLIC 110
#define PATIENT_SHMEM_OFFSET_BLOOD_PRESSURE_DIATOLIC 115
#define PATIENT_SHMEM_OFFSET_TEMPERATURE 120
#define PATIENT_SHMEM_OFFSET_RESPIRATION 130
#define PATIENT_SHMEM_OFFSET_OXYGEN_SATURATION 140

#define GET_SHMEM_MSG_TYPE (_IO_MAX+200)

// ask for a shared_mem_bytes sized object
typedef struct get_shmem_msg {
	uint16_t type;
	unsigned shared_mem_bytes;
} get_shmem_msg_t;

// response provides a handle to that object
typedef struct get_shmem_resp {
	shm_handle_t mem_handle;
} get_shmem_resp_t;



void write_shmem_int(void* shmem_ptr, int int_val, uint16_t offset, uint8_t range);

void write_shmem_float(void* shmem_ptr, float float_val, uint16_t offset, uint8_t range);

void write_shmem(void* shmem_ptr, char* output, uint16_t offset, uint8_t range);
