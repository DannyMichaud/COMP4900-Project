typedef struct monitor_data {
	int id;
	uint8_t inUse;
	void* shmem_ptr;
} monitor_data_t;

typedef struct monitor_array {
	int length;
	monitor_data_t* monitors;
} monitor_array_t;
