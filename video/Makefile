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

LIB_TARGET = video.a
OBJS =  video.o stream.o

SUB_DIRS =

.PHONY : all
all: $(OBJS)

video.o         : video.c
stream.o	: stream.c

include $(NVS_PATH)/Rules.make
