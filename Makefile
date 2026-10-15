CC     ?= cc
RM     ?= rm -f

PKGS    = glib-2.0 gdk-pixbuf-2.0 libnotify
CFLAGS  += -Wall -Wextra -std=c11 -O2
CFLAGS  += `pkg-config --cflags $(PKGS)`
LDLIBS  += `pkg-config --libs $(PKGS)`

SRC     = et.c
OBJ     = $(SRC:.c=.o)
OUTPUT  = et

.PHONY: clean

$(OUTPUT): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $(OBJ) $(LDLIBS)

clean:
	$(RM) $(OUTPUT) $(OBJ)
