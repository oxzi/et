CC     ?= gcc
CPKG    = `pkg-config --cflags glib-2.0 gdk-pixbuf-2.0 --libs libnotify`
CFLAGS  = -Wall -Wextra -pedantic --std=c11 -Ofast $(CPKG)
SRC     = et.c
OBJ     = $(SRC:.c=.o)
OUTPUT  = et

default:
	$(CC) -c $(SRC) $(CFLAGS)
	$(CC) -o $(OUTPUT) $(OBJ) $(CFLAGS)

clean:
	$(RM) $(OUTPUT) $(OBJ)
