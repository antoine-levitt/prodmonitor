OBJS = libwindowswitchs.o
LIBS =
CFLAGS = -Wall -Wextra -Wno-unused `pkg-config --cflags --libs libwnck-1.0`

CC = gcc


all: prod

$(OBJS): %.o: %.c %.h
	$(CC) -c $<  $(CFLAGS)

prod: prod.c $(OBJS)
	$(CC) $^ $(CFLAGS) -o prod

clean:
	rm -f *.o prod *~
