CC     = gcc
CPKG   = `pkg-config --cflags glib-2.0 gdk-pixbuf-2.0 --libs libnotify`
CFLAGS = -Wall -Wextra -pedantic --std=c11 -Ofast $(CPKG)
SRC    = et.c
OBJ    = $(SRC:.c=.o)
OUTPUT = et
NIXPKG = pkgconfig glib.dev

default:
	$(CC) $(CFLAGS) -c $(SRC)
	$(CC) $(CFLAGS) -o $(OUTPUT) $(OBJ)

nix:
	nix-shell -p $(NIXPKG) --command make

clean:
	$(RM) $(OUTPUT) $(OBJ)
