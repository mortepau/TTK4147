TARGET = exercise6

CFLAGS = -g -Wall -std=gnu11

LDFLAGS = -g -lpthread -lcomedi -lm -fsanitize=thread

SOURCES = $(shell find -name "*.c")

all: $(TARGET)

OBJECTS = $(SOURCES:.c=.o)

$(TARGET): $(OBJECTS)
	clang $(LDFLAGS) -o $@ $^

%.o : %.c
	clang $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) $(OBJECTS)
