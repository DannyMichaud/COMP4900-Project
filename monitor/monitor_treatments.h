#include "../patient/patient_vitals.h"

//treatment type from
typedef enum {
	IV_FLUID = 0,
	MEDICINE = 1,
} treatmentType_t;


//initialize treatment values to defaults
void initTreatments(void*);

//update the treatment administered to the patient
void updateTreatment(void*, treatmentType_t, patient_vitals_t*);
