all: prod

prod: prod.c
	gcc prod.c -o prod -Wall -Wextra -Wno-unused `pkg-config --cflags --libs libwnck-1.0`

clean:
	rm prod
