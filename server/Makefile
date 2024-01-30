CC := gcc
C_FLAG = -g -Wall
DEBUG_C_FLAG = -g -Wall -Werror
TARGET = aesdsocket.c
OBJECT = aesdsocket.o
EXE = aesdsocket

CROSS_COMPILE = aarch64-none-linux-gnu
DEBUG = N

ifeq ($(CROSS_COMPILE), aarch64-none-linux-gnu-)
	CC = $(CROSS_COMPILE)gcc
endif

ifeq ($(DEBUG), Y)
	C_FLAG = $(DEBUG_C_FLAG)
endif

# dependent to object file, then compile object to binary
all:$(OBJECT)
	$(CC) -o $(EXE) $(OBJECT)
# compile c file to object, -c means only produce object file
$(OBJECT):$(TARGET)
	$(CC) $(C_FLAG) -c $^ -o $@

clean:
	rm -f *.o $(EXE)