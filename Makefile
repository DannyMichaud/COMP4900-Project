
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
SHMEM_DIR = ./shared_memory/

CFLAGS += $(DEBUG) $(TARGET) -Wall
LDFLAGS+= $(DEBUG) $(TARGET)
BINS = $(PATIENT_DIR)patient $(MONITOR_DIR)monitor
all: $(BINS)

clean:
	rm -f *.o $(BINS);
#	cd solutions; make clean

$(PATIENT_DIR)patient: $(PATIENT_DIR)patient.o $(PATIENT_DIR)patient_vitals.o $(SHMEM_DIR)shared_memory_helpers.o

monitor.o: $(MONITOR_DIR)monitor.c $(MONITOR_DIR)monitor.h $(SHMEM_DIR)shared_memory.h
patient.o: $(PATIENT_DIR)patient.c $(PATIENT_DIR)patient.h $(PATIENT_DIR)patient_vitals.h $(SHMEM_DIR)shared_memory.h
patient_vitals.o: $(PATIENT_DIR)patient_vitals.c $(PATIENT_DIR)patient.h $(PATIENT_DIR)patient_vitals.h $(SHMEM_DIR)shared_memory.h
shared_memory_helpers.o: $(SHMEM_DIR)shared_memory_helpers.c $(SHMEM_DIR)shared_memory.h

