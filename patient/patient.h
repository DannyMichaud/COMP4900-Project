
#include "shared_memory.h"

//the type of vital we are updating
typedef enum {
	HEARTBEAT = 0,
	BLOOD_PRESSURE_SYSTOLIC = 1,
	BLOOD_PRESSURE_DIATOLIC = 2,
	TEMPERATURE = 3,
	RESPIRATION = 4,
	OXYGEN_SATURATION = 5,
} vitalType_t;

//provides info to updating thread
typedef struct patient_vital_thrinfo {
	vitalType_t vitalType;
	uint16_t offset;
} patient_vital_thrinfo_t;

typedef union {
	uint16_t int_val;
	float float_val;
} vital_val_t;

//start the patient server, given the provided server name
void startPatientServer(char* patientServerName);


//updates the vital thread
void updateVitalOnInterval(patient_vital_thrinfo_t* vitalInfo);



