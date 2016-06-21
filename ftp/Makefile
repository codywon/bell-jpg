
export CROSS:=mipsel-linux-
export CC:=$(CROSS)gcc
export AR:=$(CROSS)ar
export STRIP:=$(CROSS)strip

OSDRV:= 
LIBS := 

INC_PATH:=-I./

CFLAGS := -Wall -g $(INC_PATH) -D__MY_CGI__

SRCS    :=  $(wildcard *.c)
OBJS    :=  $(SRCS:%.c=%.o)

TARGET := ftp_thread \

.PHONY : clean all

$(TARGET):
	$(CC) $(CFLAGS) -lm -o ftp_thread ftp_thread.c ipc.c  $(LIBS) -lpthread
	$(STRIP) ftp_thread
	cp ftp_thread /tftpboot
clean:
	@rm -f $(TARGET)
	@rm -f $(OBJS)
	@rm -f *.cgi

