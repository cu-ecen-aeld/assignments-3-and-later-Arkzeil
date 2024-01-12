# default is gcc
CC := gcc
# produce debugging info
C_FLAG = -g -Wall
TARGET = writer.c
OBJECT = wrter.o
EXE = writer
# need this one
CROSS_COMPILE = aarch64-none-linux-gnu-

# if user provide CROSS_COMPILE=aarch64-none-linux-gnu-
# must have a space after ifeq
ifeq ($(CROSS_COMPILE), aarch64-none-linux-gnu-)
	CC = aarch64-none-linux-gnu-gcc
endif

# dependent to object file, then compile object to binary
all:$(OBJECT)
	$(CC) -o $(EXE) $(OBJECT)

# compile c file to object, -c means only produce object file
$(OBJECT):$(TARGET)
	$(CC) $(C_FLAG) -c $^ -o $@

clean:
	rm -f *.o $(EXE) *.elf *.map