#include "hospital_system_data.h"
#include "hospital_system_messages.h"


void startServer(name_attach_t**);

void mainLoop(name_attach_t**);

int handlePatientMessage(hospital_system_msg_to_t*, hospital_system_msg_from_t*, monitor_array_t*);

void handleMonitorMessage(hospital_system_msg_to_t*, hospital_system_msg_from_t*, monitor_array_t*);
