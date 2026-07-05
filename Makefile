# termporary test for compilation

# allows to explicitey state the compiler used by the library
CC = gcc

# includes the library compilation config
include clib.mk

# compiles the static and shared library, the variables are an absolute path to the library
all: $(CLIB_STATIC_LIB) $(CLIB_SHARED_LIB)

.PHONY: all
