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

LIB_TARGET = web.a
OBJS = asp.o balloc.o base64.o cgi.o default.o ejlex.o ejparse.o form.o h.o handler.o mime.o misc.o \
	page.o ringq.o rom.o sock.o sockGen.o security.o sym.o uemf.o url.o value.o md5c.o um.o umui.o \
	websda.o emfdb.o webrom.o webs.o websuemf.o web.o 


SUB_DIRS =

.PHONY : all
all: $(OBJS)

asp.o 		:asp.c		./webs.h ./wsIntrn.h ./ej.h ./ejIntrn.h ./uemf.h
balloc.o 	:balloc.c
base64.o 	:base64.c
cgi.o 		:cgi.c
default.o	:default.c
ejlex.o 	:ejlex.c
ejparse.o 	:ejparse.c
form.o		:form.c
h.o 		:h.c
handler.o 	:handler.c
mime.o 		:mime.c
misc.o 		:misc.c
page.o		:page.c
ringq.o 	:ringq.c
rom.o		:rom.c
sock.o 		:sock.c
sockGen.o	:sockGen.c
security.o 	:security.c
sym.o 		:sym.c
uemf.o 		:uemf.c
url.o 		:url.c
value.o		:value.c
md5c.o 		:md5c.c
um.o 		:um.c
umui.o 		:umui.c
websda.o 	:websda.c
emfdb.o		:emfdb.c
webrom.o 	:webrom.c
webs.o 		:webs.c
websuemf.o 	:websuemf.c
web.o		:web.c

include $(NVS_PATH)/Rules.make
