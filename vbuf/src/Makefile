all:	compile 

ARCH	= libvbuf.a

CC=/opt/buildroot-gcc342/bin/mipsel-linux-gcc
AR=/opt/buildroot-gcc342/bin/mipsel-linux-ar


DEPEND_FILES	= vbuf.o abuf.o mjpgbuf.o subbuf.o
				  
CFLAGS	= -DLINUX
CFLAGS += -DRT5350 
DEBUG	= -Wall -O2
IFLAGS 	= -I./ -I../../include
LDFLAGS	= 

compile: $(ARCH)

$(ARCH): $(DEPEND_FILES)
	$(AR) $(ARFLAGS) $(ARCH) $?
	cp -f $(ARCH) ../lib
	cp -f *.h ../include

clean:
	rm -f $(NAME) $(ARCH) $(DEPEND_FILES)
	rm -f ../lib/$(ARCH)

vbuf.o:  vbuf.c
abuf.o:  abuf.c
mjpgbuf.o: mjpgbuf.c
subbuf.o: subbuf.c

.c.o:
	$(CC) -c -o $@ $(DEBUG) $(CFLAGS) $(IFLAGS) $<
