# Makefile for arith (Comp 40 Assignment 4)

############## Variables ###############

CC = gcc # The compiler being used

# Updating include path to use Comp 40 .h files and CII interfaces
IFLAGS = -I/comp/40/build/include -I/usr/sup/cii40/include/cii

# Compile flags
# Set debugging information, allow the c99 standard,
# max out warnings, and use the updated include path
# CFLAGS = -g -std=c99 -Wall -Wextra -Werror -Wfatal-errors -pedantic $(IFLAGS)
# 
CFLAGS = -g -std=gnu99 -Wall -Wextra -Werror -Wfatal-errors -pedantic $(IFLAGS)

# Linking flags
# Set debugging information and update linking path
# to include course binaries and CII implementations
LDFLAGS = -g -L/comp/40/build/lib -L/usr/sup/cii40/lib64 

LDLIBS = -larith40 -l40locality -lnetpbm -lcii40 -lm -lrt

INCLUDES = $(shell echo *.h)

############### Rules ###############

all: ppmdiff 40image testbitpack 40image-6


## Compile step (.c files -> .o files)

# To get *any* .o file, compile its .c file with the following rule.
%.o: %.c $(INCLUDES)
	$(CC) $(CFLAGS) -c $< -o $@

## Linking step (.o -> executable program)

ppmdiff: ppmdiff.o a2blocked.o uarray2b.o uarray2.o
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

40image: 40image.o compress40.o a2blocked.o a2plain.o uarray2b.o uarray2.o arith_helper.o bitpack.o
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

testbitpack: testbitpack.o bitpack.o 
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

40image-6: 40image.o compress40.o a2blocked.o a2plain.o uarray2b.o uarray2.o arith_helper.o bitpack.o
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

clean:
	rm -f ppmdiff 40image testbitpack 40image-6 *.o

