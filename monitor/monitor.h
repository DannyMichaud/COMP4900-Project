void connectToHospitalSystem();

// Connects monitor to a given patient
void connectToPatient(char* patientServerName);

// Check patient vitals
void monitorPatientVitals(void* shmem_ptr);
