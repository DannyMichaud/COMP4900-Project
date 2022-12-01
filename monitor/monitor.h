void connectToHospitalSystem();

// monitors the shared memory with the hospital system
void monitorHospitalSystemSharedMemory(shm_handle_t handle);

// Connects monitor to a given patient
void connectToPatient(char* patientServerName);

// Check patient vitals
void monitorPatientVitals(void* shmem_ptr, int int_coid);
