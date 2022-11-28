#define HOSPITAL_SERVER_NAME "hospitalSystem"

void startServer(name_attach_t**);

void mainLoop(name_attach_t**);

void handlePatientMessage(hospital_system_msg_to_t*, hospital_system_msg_from_t*, monitor_array_t*);

void handleServerMessage(hospital_system_msg_to_t*, hospital_system_msg_from_t*, monitor_array_t*);
