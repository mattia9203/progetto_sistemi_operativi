CC=gcc
CCOPTS=--std=gnu99 -Wall -D_LIST_DEBUG_ 
AR=ar

OBJS=Bitmap.o\
     buddy_allocator.o\
	 main_malloc.o

HEADERS=Bitmap.h buddy_allocator.h main_malloc.h

LIBS=libbuddy.a

BINS= main_test buddy_test

.phony: clean all


all:	$(LIBS) $(BINS)

%.o:	%.c $(HEADERS)
	$(CC) $(CCOPTS) -c -o $@  $<

libbuddy.a: $(OBJS) 
	$(AR) -rcs $@ $^
	$(RM) $(OBJS)


main_test: main_test.o $(LIBS)
	$(CC) $(CCOPTS) -o $@ $^ -lm
	
buddy_test: buddy_test.o $(LIBS)
	$(CC) $(CCOPTS) -o $@ $^ -lm

clean:
	rm -rf *.o *~ $(LIBS) $(BINS)