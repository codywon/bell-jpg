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

LIB_TARGET = wm8750.a
OBJS =  wm8750.o wm8751_ctrl.o

SUB_DIRS =

.PHONY : all
all: $(OBJS)

wm8750.o		: wm8750.c
wm8751_ctrl.o : wm8751_ctrl.c

include $(NVS_PATH)/Rules.make
