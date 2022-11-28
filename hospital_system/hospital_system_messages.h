
/* For messages exchanged between patient and hospital system, monitor and hospital system */

//whether the message came from a patient or monitor
typedef enum {
	SOURCE_PATIENT = 0,
	SOURCE_MONITOR = 1,
} hs_msgSource_t;

//type of message being sent
typedef enum {
	HS_MSG_CONNECT = 0, //connection request
} hs_msgType_t;

//type of reply being sent
typedef enum {
	HS_REPLY_SERVER_NAME = 0, //for sending server id to be used by patient
	HS_REPLY_ID = 1, //
} hs_msgReply_t;

//for message sent to hospital server
typedef struct hospital_system_msg_to {
	hs_msgSource_t messageSender;
	hs_msgType_t messageType;
	int id;
} hospital_system_msg_to_t;

//data for message received from hospital server
typedef union hospital_system_msg_data {
	int int_data;
	float float_data;
	char string_data[30];
} hospital_system_msg_data_t;

//for message received from hospital server
typedef struct hospital_system_msg_from {
	hs_msgReply_t messageReplyType;
	hospital_system_msg_data_t data;
} hospital_system_msg_from_t;

