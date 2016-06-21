#
# This file contains rules which are shared between multiple Makefiles
# in DVR system project.
#
# Author: ROADJUN
# Creation Date: 2009-04-04
# Modifier:
# Last Modification Date: 
# 
#

#
# Common rules
#

.PHONY : all
all: $(OBJS) $(OBJ_TARGET) $(LIB_TARGET) subdirs


$(OBJS):
	@echo "compile $< file ..."
    ifndef MOD_TARGET
	@$(CC) $(GDB) $(CFLAGS)   -c -o $@ $<
    else
	@$(CC) $(GDB) $(MODFLAGS)  -c -o $@ $<
    endif


#
# Rule to compile a set of .o files into one .o file
#

ifdef OBJ_TARGET
$(OBJ_TARGET): $(OBJS)
    ifneq "$(strip $(OBJS))" ""
	@echo "create one OBJ $@ ..."
	@$(LD) $(LDFLAGS) -r -o $@ $^
    endif
endif

#
# Rule to compile a set of .o files into one .a file
#

ifdef LIB_TARGET
$(LIB_TARGET): $(OBJS)
    ifneq "$(strip $(OBJS))" ""
	@echo "create library $@ ..."
	@$(AR) $(ARFLAGS) rcs $@ $^ 
    endif
endif

	
#
# Rule to make subdirectories
#

.PHONY : subdirs
subdirs:
ifdef SUB_DIRS
    ifneq "$(strip $(SUB_DIRS))" ""
	@( \
	for dir in $(SUB_DIRS); do \
	  $(MAKE) -C $$dir; \
	done; \
	)
    endif
endif


.PHONY : clean
clean:
    ifneq "$(strip $(OBJS))" ""
	rm -f $(OBJS)
	rm -f $(OBJ_TARGET) $(LIB_TARGET) $(MOD_TARGET)
    endif
    ifneq "$(strip $(SUB_DIRS))" ""
	@( \
	for dir in $(SUB_DIRS); do \
	  $(MAKE) -C $$dir clean; \
	done; \
	)
    endif
