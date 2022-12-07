
#
#	Makefile for 4900 project
#

DEBUG = -g
CC = qcc
LD = qcc


TARGET = -Vgcc_ntox86_64
#TARGET = -Vgcc_ntox86
#TARGET = -Vgcc_ntoarmv7le
#TARGET = -Vgcc_ntoaarch64le

MONITOR_DIR = ./monitor/
PATIENT_DIR = ./patient/
PATIENT_I_DIR = ./patient_initializer/
HOSPITAL_SYSTEM_DIR = ./hospital_system/
SHMEM_DIR = ./shared_memory/
SHUTDOWN_DIR = ./system_shutdown/

CFLAGS += $(DEBUG) $(TARGET) -Wall
LDFLAGS+= $(DEBUG) $(TARGET)
BINS = $(PATIENT_DIR)patient $(MONITOR_DIR)monitor $(HOSPITAL_SYSTEM_DIR)hospital_system $(SHUTDOWN_DIR)shutdown $(PATIENT_I_DIR)patient_initializer
all: $(BINS)

clean:
	rm -f *.o $(BINS);
#	cd solutions; make clean

$(PATIENT_DIR)patient: $(PATIENT_DIR)patient.o $(PATIENT_DIR)patient_vitals.o $(SHMEM_DIR)shared_memory_helpers.o

$(MONITOR_DIR)monitor: $(MONITOR_DIR)monitor.o $(MONITOR_DIR)monitor_treatments.o $(SHMEM_DIR)shared_memory_helpers.o

$(HOSPITAL_SYSTEM_DIR)hospital_system: $(HOSPITAL_SYSTEM_DIR)hospital_system.o $(SHMEM_DIR)shared_memory_helpers.o

$(PATIENT_I_DIR)patient_initializer: $(PATIENT_I_DIR)patient_initializer.o

$(SHUTDOWN_DIR)shutdown: $(SHUTDOWN_DIR)shutdown.o

hospital_system.o: $(HOSPITAL_SYSTEM_DIR)hospital_system.c $(HOSPITAL_SYSTEM_DIR)hospital_system.h $(HOSPITAL_SYSTEM_DIR)hospital_system_data.h $(HOSPITAL_SYSTEM_DIR)hospital_system_messages.h \
		$(HOSPITAL_SYSTEM_DIR)hospital_system_address.h $(SHMEM_DIR)shared_memory.h
monitor.o: $(MONITOR_DIR)monitor.c $(MONITOR_DIR)monitor.h $(MONITOR_DIR)monitor_internal_messaging.h $(SHMEM_DIR)shared_memory.h $(PATIENT_DIR)patient_vitals.h $(HOSPITAL_SYSTEM_DIR)hospital_system_messages.h  $(HOSPITAL_SYSTEM_DIR)hospital_system_address.h
monitor_treatments.o: $(MONITOR_DIR)monitor_treatments.c $(MONITOR_DIR)monitor_treatments.h $(SHMEM_DIR)shared_memory.h $(PATIENT_DIR)patient_vitals.h
patient.o: $(PATIENT_DIR)patient.c $(PATIENT_DIR)patient.h $(PATIENT_DIR)patient_vitals.h $(PATIENT_DIR)patient_vitals_utils.h $(SHMEM_DIR)shared_memory.h
patient_vitals.o: $(PATIENT_DIR)patient_vitals.c $(PATIENT_DIR)patient.h $(PATIENT_DIR)patient_vitals.h $(PATIENT_DIR)patient_vitals_utils.h $(SHMEM_DIR)shared_memory.h
shared_memory_helpers.o: $(SHMEM_DIR)shared_memory_helpers.c $(SHMEM_DIR)shared_memory.h
shutdown.o: $(SHUTDOWN_DIR)shutdown.c $(HOSPITAL_SYSTEM_DIR)hospital_system.h $(HOSPITAL_SYSTEM_DIR)hospital_system_data.h $(HOSPITAL_SYSTEM_DIR)hospital_system_messages.h $(HOSPITAL_SYSTEM_DIR)hospital_system_address.h
