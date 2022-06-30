CC=gcc
CCOPTS=--std=gnu99 -Wall
AR=ar
HEADERS=read_proc.h
OBJS=read_proc.o
LIBS=main.a
BINS=main

#disastros_test

.phony: clean all


all:	$(LIBS) $(BINS)

%.o:	%.c $(HEADERS)
	$(CC) $(CCOPTS) -c -o $@  $<

main.a: $(OBJS) 
	$(AR) -rcs $@ $^
	$(RM) $(OBJS)

main: main.c $(LIBS)
	$(CC) $(CCOPTS) -o $@ $^

clean:
	rm -rf *.o *~ $(LIBS) $(BINS)