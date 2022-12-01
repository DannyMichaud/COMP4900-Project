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
#include "../shared_memory/shared_memory.h"


void initTreatments(void* patient_shmem_ptr){
	write_shmem_int(patient_shmem_ptr, 0, PATIENT_SHMEM_OFFSET_TREATMENT_IV_FLUID, 10);
	write_shmem_int(patient_shmem_ptr, 0, PATIENT_SHMEM_OFFSET_TREATMENT_MEDICINE, 10);
}

void updateTreatment(void* patient_shmem_ptr, treatmentType_t type, patient_vitals_t* vitals){
	switch(type){
	case PATIENT_SHMEM_OFFSET_TREATMENT_IV_FLUID:
		write_shmem_int(patient_shmem_ptr, 0, PATIENT_SHMEM_OFFSET_TREATMENT_IV_FLUID, 10);
		break;
	case PATIENT_SHMEM_OFFSET_TREATMENT_MEDICINE:
		write_shmem_int(patient_shmem_ptr, 0, PATIENT_SHMEM_OFFSET_TREATMENT_MEDICINE, 10);
		break;
	default:
		break;
	}
}
