#define DEFAULT_HEARTRATE 80
#define DEFAULT_SYSTOLIC_BP 100
#define DEFAULT_DIASTOLIC_BP 70
#define DEFAULT_TEMPERATURE 36.6
#define DEFAULT_RESPIRATION 14
#define DEFAULT_OXYGEN_SATURATION 95


// struct containing all vitals of patients
typedef struct patient_vitals {
	uint8_t heartRate;
	uint8_t systolicBP;
	uint8_t diastolicBP;
	float temperature;
	uint8_t respiration;
	uint8_t oxygenSaturation;
} patient_vitals_t;

vital_val_t getVital(vitalType_t vitalType);
