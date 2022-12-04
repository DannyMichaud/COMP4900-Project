
#include "../shared_memory/shared_memory.h"

//the type of vital we are updating
typedef enum {
	HEARTBEAT = 0,
	BLOOD_PRESSURE_SYSTOLIC = 1,
	BLOOD_PRESSURE_DIASTOLIC = 2,
	TEMPERATURE = 3,
	RESPIRATION = 4,
	OXYGEN_SATURATION = 5,
} vitalType_t;

typedef struct illness {
	vitalType_t vitalType;
	uint8_t severity;
} illness_t;

typedef enum {
	HEALTHY = 0,
	UNHEALTHY = 1
} patientHealth_t;

//provides info to updating thread
typedef struct patient_vital_thrinfo {
	vitalType_t vitalType;
	uint16_t offset;
	uint8_t range;
	void* shmem_ptr;
} patient_vital_thrinfo_t;

typedef union {
	uint16_t int_val;
	float float_val;
} vital_val_t;

// start the patient server, given the provided server name
void startPatientServer(char* patientServerName);

// gets vitals on an interval and writes to shared memory
void getVitalOnInterval(patient_vital_thrinfo_t* vitalInfo);

// updates vitals on an interval
void updateVitalOnInterval();

