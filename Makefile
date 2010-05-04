all:
	gcc prod.c -o prod -lX11 -lXss -Wall -Wextra -Wno-unused `pkg-config --cflags --libs libwnck-1.0` -lsqlite3
