TARGET = exercise1

CFLAGS = -g -Wall -std=gnu11

LDFLAGS = -g

SOURCES = $(shell find -name "*.c")

all: $(TARGET)

OBJECTS = $(SOURCES:.c=.o)

$(TARGET): $(OBJECTS)
	gcc $(LDFLAGS) -o $@ $^

%.o : %.c
	gcc $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) $(OBJECTS)
