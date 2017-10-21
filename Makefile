CC     ?= gcc
CPKG    = `pkg-config --cflags glib-2.0 gdk-pixbuf-2.0 --libs libnotify`
CFLAGS  = -Wall -Wextra -pedantic --std=c11 -Ofast $(CPKG)
SRC     = et.c
OBJ     = $(SRC:.c=.o)
OUTPUT  = et

default:
	$(CC) $(CFLAGS) -c $(SRC)
	$(CC) $(CFLAGS) -o $(OUTPUT) $(OBJ)

clean:
	$(RM) $(OUTPUT) $(OBJ)
