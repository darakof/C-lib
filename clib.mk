############################################################
#                                                          #
# when used as a library and needed to compile with it,    #
# include this file in the makefile and it will create     #
# the library .a and .so will be built in                  #
# /path/to/C-lib/bin/libclib(.a/.so)                       #
# Ex: /home/user/C-lib/bin/libclib.so                      #
# the resulting library is a link to the versioned files   #
#                                                          #
############################################################

# defaults
CC ?= gcc
ARCHIVER ?= ar
CFLAGS ?= -O2

CLIB_VER = 0.1

CLIB_OBJ = array.o string.o arenamem.o heapmem.o poolmem.o string.o
CLIB_BUILD_OBJ = $(CLIB_OBJ:%=$(CLIB_BUILD_LOC)/%)

CLIB_LOC := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
CLIB_BIN_LOC = $(CLIB_LOC)/bin
CLIB_BUILD_LOC = $(CLIB_LOC)/build
CLIB_INCLUDE_LOC = $(CLIB_LOC)/include
# link against this
CLIB_LIB = clib

# actual output files with location
CLIB_STATIC_LIB_LOC = $(CLIB_BIN_LOC)/lib$(CLIB_LIB).a
CLIB_SHARED_LIB_LOC = $(CLIB_BIN_LOC)/lib$(CLIB_LIB).so

# the library path to link against
CLIB_LIB_LOC  = $(CLIB_BIN_LOC)

$(CLIB_BUILD_LOC)/:
	mkdir -p $@

$(CLIB_BIN_LOC)/:
	mkdir -p $@

$(CLIB_BUILD_LOC)/%.o: $(CLIB_LOC)/src/%.c | $(CLIB_BUILD_LOC)/
	$(CC) $(CFLAGS) -fPIC -c $< -o $@ -I$(CLIB_INCLUDE_LOC)

$(CLIB_STATIC_LIB_LOC): $(CLIB_BUILD_OBJ) | $(CLIB_BIN_LOC)/
	$(ARCHIVER) rcs $@ $^
	@echo "Static Library Built Successfully"

$(CLIB_SHARED_LIB_LOC): $(CLIB_BUILD_OBJ) | $(CLIB_BIN_LOC)/
	$(CC) -shared $^ -o $@
	@echo "Shared Library Built Successfully"

$(CLIB_LIB): $(CLIB_STATIC_LIB_LOC) $(CLIB_SHARED_LIB_LOC)

clean-clib:
	rm -fr build bin

.PHONY: clean-clib
