#
# This Makefile of audio Module in DVR project.
#
# Author: ROADJUN
# Creation Date: 2009-04-04
# Modifier:
# Last Modification Date: 
# 
#

ifeq ($(MAKELEVEL), 0)
$(error Note: This isn't a TOP Makefile)
endif

LIB_TARGET = network.a
OBJS =  ipcaudioin.o  ipcaudioout.o  ipc.o  network.o packet.o dhcp.o yinetwork.o

SUB_DIRS =

.PHONY : all
all: $(OBJS)


bcast.o  	:bcast.c
ipcaudioin.o  	:ipcaudioin.c
ipcaudioout.o  	:ipcaudioout.c
ipc.o 		:ipc.c
network.o 	:network.c
network-ap.o	: network-ap.c
packet.o	: packet.c
dhcp.o		: dhcp.c
yinetwork.o : yinetwork.c

include $(NVS_PATH)/Rules.make
