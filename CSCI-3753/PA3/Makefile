CC=gcc
CFLAGS=-Wall -Wextra -g
LDFLAGS=-pthread
SOURCES=safe_queue.c multi-lookup.c util.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=multi-lookup

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@


clean:
	rm *.o $(EXECUTABLE)
