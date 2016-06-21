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

LIB_TARGET = function.a
OBJS =  alarm.o hi_draw.o ieproc.o main.o moto-new.o rs.o authcreate.o ieparam.o init.o md5.o ntp.o \
	netcmd.o register.o rtsp.o upnp.o ddns.o log.o date.o gpio.o ftp.o encrypt.o sd.o

SUB_DIRS =

.PHONY : all
all: $(OBJS)

alarm.o		: alarm.c
hi_draw.o	: hi_draw.c
ieproc.o	: ieproc.c
main.o		: main.c
moto-new.o	: moto-new.c
proc.o		: proc.c
rs.o		: rs.c
authcreate.o: authcreate.c
ieparam.o	: ieparam.c
init.o		: init.c
md5.o		: md5.c
ntp.o		: ntp.c
netcmd.o	: netcmd.c
register.o	: register.c
rtsp.o		: rtsp.c
upnp.o		: upnp.c
mail.o		: mail.c
ddns.o		: ddns.c
log.o		: log.c
date.o		: date.c
gpio.o		: gpio.c
ftp.o		: ftp.c
update.o	: update.c
encrypt.o	: encrypt.c
sd.o        : sd.c

include $(NVS_PATH)/Rules.make
