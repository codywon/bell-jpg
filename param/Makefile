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

LIB_TARGET = param.a
OBJS =  sysparam.o

SUB_DIRS =

.PHONY : all
all: $(OBJS)

sysparam.o       : sysparam.c

include $(NVS_PATH)/Rules.make
