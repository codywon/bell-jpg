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

LIB_TARGET = dns.a
OBJS =  dns.o factory.o

SUB_DIRS =

.PHONY : all
all: $(OBJS)

dns.o		: dns.c
factory.o	: factory.c

include $(NVS_PATH)/Rules.make
