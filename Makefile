
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

CFLAGS += $(DEBUG) $(TARGET) -Wall
LDFLAGS+= $(DEBUG) $(TARGET)
BINS = $(PATIENT_DIR)patient $(MONITOR_DIR)monitor
all: $(BINS)

clean:
	rm -f *.o $(BINS);
#	cd solutions; make clean


monitor.o: $(MONITOR_DIR)monitor.c
patient.o: $(PATIENT_DIR)patient.c

